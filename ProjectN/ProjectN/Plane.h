#ifndef PLANE_H
#define PLANE_H

#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<random>

class Plane {
public:
	Mesh mesh;
	glm::vec3 color;
	Plane(){
		mesh.clear();
	}

	void Draw(Shader& shader)
	{
		mesh.Draw(shader);
	}

private:

};

class Room{
public:
	Plane walls[4], ground;
	Texture woodTexture;
	float size;
	Ball light = Ball(0.0f, 1.0f, 0.5f, 0.1f);
	//Ball light = Ball(0.0f, -0.8f, 0.0f, 0.12f);

	Room(float siz):size(siz) {
		CreateTexture();
		CreateGround();
		CreateWalls();
	}

	void Draw(Shader& pureShader, Shader& textureShader, Shader& lightShader, Shader& ceilingShader, Shader& groundShader, int depthMap) {
		pureShader.use();
		for (int i = 0; i < 3; i++) {
			pureShader.setVec3("objectColor", walls[i].color.x, walls[i].color.y, walls[i].color.z);
			walls[i].Draw(pureShader);
		}
		ceilingShader.use();
		ceilingShader.setVec3("objectColor", walls[3].color.x, walls[3].color.y, walls[3].color.z);
		walls[3].Draw(ceilingShader);
		groundShader.use();
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		ground.Draw(groundShader);
		lightShader.use();
		light.Draw(lightShader);
	}
	void renderShadow(Shader& simpleDepthShader) {
		ground.Draw(simpleDepthShader);
	}

	void CreateWalls() {
		Vertex vertex[4];

		vertex[0].Position = glm::vec3(size, size, -size);
		vertex[0].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex[1].Position = glm::vec3(-size, size, -size);
		vertex[1].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex[2].Position = glm::vec3(-size, -size, -size);
		vertex[2].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex[3].Position = glm::vec3(size, -size, -size);
		vertex[3].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		walls[0].mesh.vertices.push_back(vertex[0]);
		walls[0].mesh.vertices.push_back(vertex[1]);
		walls[0].mesh.vertices.push_back(vertex[2]);
		walls[0].mesh.vertices.push_back(vertex[3]);
		walls[0].mesh.indices.push_back(0);
		walls[0].mesh.indices.push_back(1);
		walls[0].mesh.indices.push_back(2);
		walls[0].mesh.indices.push_back(0);
		walls[0].mesh.indices.push_back(2);
		walls[0].mesh.indices.push_back(3);
		walls[0].mesh.setup();
		walls[0].color = glm::vec3(0.05859375f, 0.734375f, 0.97265625f);

		vertex[0].Position = glm::vec3(-size, size, -size);
		vertex[0].Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex[1].Position = glm::vec3(-size, size, size);
		vertex[1].Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex[2].Position = glm::vec3(-size, -size, size);
		vertex[2].Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex[3].Position = glm::vec3(-size, -size, -size);
		vertex[3].Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		walls[1].mesh.vertices.push_back(vertex[0]);
		walls[1].mesh.vertices.push_back(vertex[1]);
		walls[1].mesh.vertices.push_back(vertex[2]);
		walls[1].mesh.vertices.push_back(vertex[3]);
		walls[1].mesh.indices.push_back(0);
		walls[1].mesh.indices.push_back(1);
		walls[1].mesh.indices.push_back(2);
		walls[1].mesh.indices.push_back(0);
		walls[1].mesh.indices.push_back(2);
		walls[1].mesh.indices.push_back(3);
		walls[1].mesh.setup();
		walls[1].color = glm::vec3(0.04296875f, 0.90625f, 0.50390625f);

		vertex[0].Position = glm::vec3(size, size, -size);
		vertex[0].Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex[1].Position = glm::vec3(size, size, size);
		vertex[1].Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex[2].Position = glm::vec3(size, -size, size);
		vertex[2].Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex[3].Position = glm::vec3(size, -size, -size);
		vertex[3].Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		walls[2].mesh.vertices.push_back(vertex[0]);
		walls[2].mesh.vertices.push_back(vertex[1]);
		walls[2].mesh.vertices.push_back(vertex[2]);
		walls[2].mesh.vertices.push_back(vertex[3]);
		walls[2].mesh.indices.push_back(0);
		walls[2].mesh.indices.push_back(1);
		walls[2].mesh.indices.push_back(2);
		walls[2].mesh.indices.push_back(0);
		walls[2].mesh.indices.push_back(2);
		walls[2].mesh.indices.push_back(3);
		walls[2].mesh.setup();
		walls[2].color = glm::vec3(0.93359375f, 0.33984375f, 0.46484375f);

		vertex[0].Position = glm::vec3(size, size, -size);
		vertex[0].Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex[1].Position = glm::vec3(-size, size, -size);
		vertex[1].Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex[2].Position = glm::vec3(-size, size, size);
		vertex[2].Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex[3].Position = glm::vec3(size, size, size);
		vertex[3].Normal = glm::vec3(0.0f,-1.0f, 0.0f);
		walls[3].mesh.vertices.push_back(vertex[0]);
		walls[3].mesh.vertices.push_back(vertex[1]);
		walls[3].mesh.vertices.push_back(vertex[2]);
		walls[3].mesh.vertices.push_back(vertex[3]);
		walls[3].mesh.indices.push_back(0);
		walls[3].mesh.indices.push_back(1);
		walls[3].mesh.indices.push_back(2);
		walls[3].mesh.indices.push_back(0);
		walls[3].mesh.indices.push_back(2);
		walls[3].mesh.indices.push_back(3);
		walls[3].mesh.setup();
		walls[3].color = glm::vec3(0.8203125f, 0.8515625f, 0.8828125f);

	}
	void CreateGround() {
		Vertex vertex[4];

		vertex[0].Position = glm::vec3(size, -size, -size);
		vertex[0].Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex[0].TexCoords = glm::vec2(1.0f, 1.0f);
		vertex[1].Position = glm::vec3(-size, -size, -size);
		vertex[1].Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex[1].TexCoords = glm::vec2(0.0f, 1.0f);
		vertex[2].Position = glm::vec3(-size, -size, size);
		vertex[2].Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex[2].TexCoords = glm::vec2(0.0f, 0.0f);
		vertex[3].Position = glm::vec3(size, -size, size);
		vertex[3].Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex[3].TexCoords = glm::vec2(1.0f, 0.0f);

		ground.mesh.vertices.push_back(vertex[0]);
		ground.mesh.vertices.push_back(vertex[1]);
		ground.mesh.vertices.push_back(vertex[2]);
		ground.mesh.vertices.push_back(vertex[3]);
		ground.mesh.indices.push_back(0);
		ground.mesh.indices.push_back(1);
		ground.mesh.indices.push_back(2);
		ground.mesh.indices.push_back(0);
		ground.mesh.indices.push_back(2);
		ground.mesh.indices.push_back(3);

		ground.mesh.textures.push_back(woodTexture);
		ground.mesh.setup();
	}

	void CreateTexture() {

		unsigned int textureID;
		glGenTextures(1, &textureID);
		int width, height, nrComponents;
		unsigned char* data = stbi_load("texture\\wood.jpg", &width, &height, &nrComponents, 0);
		if (data)
		{

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: ";
			stbi_image_free(data);
		}
		woodTexture.id = textureID;
		woodTexture.type = "texture_diffuse";
		woodTexture.path = "texture\\wooden.jpg";
	};
};
#endif