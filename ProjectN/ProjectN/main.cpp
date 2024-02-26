#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "tumbler.h"
#include "SELFUTILS.h"
#include "Plane.h"
#include "FireAnimation.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
void renderQuad();

// settings
const unsigned int SCR_WIDTH = 1500;
const unsigned int SCR_HEIGHT = 1500;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.6f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// animate relative
bool ballMoving = false;
bool hasMousePressed = false;
int MouseControllingIdx = -1;

// Objects
TumblerCluster tumblers;
BallSystem ballSys;
FireBall fireBall;
StaticParticleManager ptm;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);


    // tell GLFW to capture our mouse
   // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // -----------------------------------------------------------------------------------------------------------------------------------------------------------
    // Prebuilding Configurations
    // -----------------------------------------------------------------------------------------------------------------------------------------------------------

    // Shaders Configurations
    Shader pureShader("shader\\pure.vs", "shader\\pure.fs");
    Shader ceilingShader("shader\\ceiling.vs", "shader\\ceiling.fs");
    Shader textureShader("shader\\texture.vs", "shader\\texture.fs");
    Shader lightShader("shader\\light.vs", "shader\\light.fs");
    Shader tumblerShader("shader\\texture.vs", "shader\\texture.fs");
    Shader groundShader("shader\\ground.vs", "shader\\ground.fs");
    Shader ballShader("shader\\ball.vs", "shader\\ball.fs");
    Shader particleShader("shader\\particle.vs", "shader\\particle.fs");

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    InitShader(pureShader, projection, view, camera.Position);

    InitShader(ceilingShader, projection, view, camera.Position);
    ceilingShader.setVec3("light.ambient", 0.7f, 0.7f, 0.7f);

    InitShader(textureShader, projection, view, camera.Position);
    textureShader.setVec3("light.specular", 0.9f, 0.9f, 0.9f);

    InitShader(ballShader, projection, view, camera.Position);

    InitShader(lightShader, projection, view, camera.Position);

    InitShader(tumblerShader, projection, view, camera.Position);

    InitShader(groundShader, projection, view, camera.Position);

    InitShader(particleShader, projection, view, camera.Position);

    // Shadow texture
    Shader debugDepthQuad("shader\\debug_quad_depth.vs", "shader\\debug_quad_depth.fs");
    Shader simpleDepthShader("shader\\shadow_mapping_depth.vs", "shader\\shadow_mapping_depth.fs");
    // configure depth map FBO
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // shader configuration
    groundShader.use();
    groundShader.setInt("shadowMap", 9);
    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 9);




    // -----------------------------------------------------------------------------------------------------------------------------------------------------------
    // Prebuilding Objects
    // -----------------------------------------------------------------------------------------------------------------------------------------------------------

    // Room and tumblers
    Room room(1.0f);
    tumblers.Init();
    ballSys.InitBalls();



    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        tumblers.KineticCalculation(deltaTime);
        ballSys.Animate(deltaTime);
        fireBall.Update(deltaTime);
        ptm.Update(deltaTime);
        Balls_CollideCalculation(ballSys, room, tumblers);
        Fireball_CollideCalculation(fireBall, ballSys, room, tumblers, ptm);

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
       

        // -----------------------------------------------------------------------------------------------------------------------------------------------------------
        // Generating Shadow Map
        // -----------------------------------------------------------------------------------------------------------------------------------------------------------

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 0.1f, far_plane = 3.0f;
        lightProjection = glm::perspective(glm::radians(100.0f), 1.0f, near_plane, far_plane);
        lightView = glm::lookAt(glm::vec3(0.0f, 1.0f, 0.5f), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        // render scene from light's point of view
        groundShader.use();
        groundShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, woodTexture);
        tumblers.renderShadow(simpleDepthShader);
        ballSys.renderShadow(simpleDepthShader);
        fireBall.renderShadow(simpleDepthShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // -----------------------------------------------------------------------------------------------------------------------------------------------------------
        // Drawing Objects
        // -----------------------------------------------------------------------------------------------------------------------------------------------------------
        
        // Update if moving is allowed
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        InitShader(pureShader, projection, view, camera.Position);
        InitShader(ceilingShader, projection, view, camera.Position);
        ceilingShader.setVec3("light.ambient", 0.7f, 0.7f, 0.7f);
        InitShader(textureShader, projection, view, camera.Position);
        textureShader.setVec3("light.specular", 0.9f, 0.9f, 0.9f);
        InitShader(lightShader, projection, view, camera.Position);
        InitShader(tumblerShader, projection, view, camera.Position);
        InitShader(groundShader, projection, view, camera.Position);
        InitShader(ballShader, projection, view, camera.Position);
        InitShader(particleShader, projection, view, camera.Position);

        // Update fireBall position to shaders;
        UpdateFireballToShader(pureShader, fireBall);
        UpdateFireballToShader(ceilingShader, fireBall);
        UpdateFireballToShader(textureShader, fireBall);
        UpdateFireballToShader(tumblerShader, fireBall);
        UpdateFireballToShader(groundShader, fireBall);
        UpdateFireballToShader(ballShader, fireBall);
        // Draw room and tumblers
        room.Draw(pureShader, textureShader, lightShader, ceilingShader, groundShader, depthMap);
        tumblers.Draw(tumblerShader);

        // Draw Balls
        ballSys.Draw(ballShader);

        // Draw Fire Animations
        fireBall.Draw(lightShader, particleShader);

        // Draw animation particles
        ptm.Draw(particleShader);


        // render Depth map to quad for visual debugging
        // ---------------------------------------------
        //debugDepthQuad.use();
        //debugDepthQuad.setFloat("near_plane", near_plane);
        //debugDepthQuad.setFloat("far_plane", far_plane);
        //glActiveTexture(GL_TEXTURE9);
        //glBindTexture(GL_TEXTURE_2D, depthMap);
        //renderQuad();

        // -----------------------------------------------------------------------------------------------------------------------------------------------------------
        // Finish Drawing Objects
        // -----------------------------------------------------------------------------------------------------------------------------------------------------------
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
bool isKeyXPressed = false;
bool isKeyFPressed = false;
bool isKeyPPressed = false;
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (!isKeyXPressed)ballSys.Activate();
        isKeyXPressed = true;
    }
    else if(glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE)
        isKeyXPressed = false;


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessMouseMovement(0.0f, 0.5f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessMouseMovement(0.0f, -0.5f);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessMouseMovement(-0.5f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessMouseMovement(0.5f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        ballMoving = true;


    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !isKeyFPressed) {
        isKeyFPressed = true;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float ndcX = (2.0f * xpos) / SCR_WIDTH - 1.0f;
        float ndcY = 1.0f - (2.0f * ypos) / SCR_HEIGHT;

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0, 1.0);
        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

        glm::vec4 rayWorld = glm::inverse(view) * rayEye;
        glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));

        fireBall.Launch(camera.Position, rayDirection);
        //fireBall.Launch(glm::vec3(0.0f), glm::vec3(1.0f,0.0f,0.0f));
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)isKeyFPressed = false;

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !isKeyPPressed) {
        isKeyPPressed = true;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float ndcX = (2.0f * xpos) / SCR_WIDTH - 1.0f;
        float ndcY = 1.0f - (2.0f * ypos) / SCR_HEIGHT;

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0, 1.0);
        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

        glm::vec4 rayWorld = glm::inverse(view) * rayEye;
        glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));

        //ballSys.Debug(camera.Position, rayDirection);
        //ptm.SE_Ash(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ptm.SE_Sparkle(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    }
    else if(glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)isKeyPPressed = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

   // camera.ProcessMouseMovement(xoffset, yoffset);


    //---------------------------------------------------------------------------------------------------
    // tumbler controll
    //---------------------------------------------------------------------------------------------------

    if (hasMousePressed && MouseControllingIdx != -1) {
        tumblers.processOffsets(xoffset, yoffset);
    }

}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (hasMousePressed)return;

        hasMousePressed = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        //---------------------------------------------------------------------------------------------------
        // check ray
        //---------------------------------------------------------------------------------------------------

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float ndcX = (2.0f * xpos) / SCR_WIDTH - 1.0f;
        float ndcY = 1.0f - (2.0f * ypos) / SCR_HEIGHT;

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0, 1.0);
        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

        glm::vec4 rayWorld = glm::inverse(view) * rayEye;
        glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));

        MouseControllingIdx = tumblers.checkRay(camera.Position, rayDirection);
        printf("touched %d.\n", MouseControllingIdx);
        if(MouseControllingIdx == -1){
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        hasMousePressed = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        tumblers.ReleaseMouse();
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}