#pragma once
#ifndef BALL_H
#define BALL_H

#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Randomizer.h"

const float pi = 3.1415926535897932384626433832795;
enum dType { Default, Wall0, Wall1, Wall2, Wall3, Ground, Tumblers};
class Ball
{
public:
	Mesh mesh;
	glm::vec3 V;
	glm::vec3 G = glm::vec3(0.0f, -0.98f, 0.0f);
	glm::vec3 position;
	float radius;

	bool living = true;
	const float destroyLimit = 4.0f;
	const float EnergyLossPerCollide = 0.000f;
	const float mass = 0.1f;

	dType displayType;

	Ball() { living = false; }
	void initParam(float x, float y, float z, float r){
		radius = r;
		living = false;
		position = glm::vec3(x, y, z);
		V = glm::vec3(0.0f);
		displayType = Default;
		GenerateMesh();
	}

	Ball(float x, float y, float z, float r) :radius(r) {
		living = true;
		position = glm::vec3(x, y, z);
		V = glm::vec3(0.0f);
		GenerateMesh();
	}


	void KineticMove(float deltaTime) {
		if (!living)return;
		position += V * deltaTime;
		V += G * deltaTime;


		if (abs(position.x) >= destroyLimit || abs(position.y) >= destroyLimit || abs(position.z) >= destroyLimit)
			living = false;
		//if (!glm::length(V) && position.z < -0.9f)
		//	living = false;
	}

	void Reflect(glm::vec3 normal) {
		V = V - 2 * glm::dot(V, normal) * normal;
		float E1 = 1.0f / 2 * mass * glm::dot(V, V);
		float E2 = glm::max(0.0f, E1 - EnergyLossPerCollide);
		float ratio = glm::sqrt(E2 * 2.0f / mass) / glm::length(V);
		V *= ratio;
		position += V * 0.1f;
	}

	void GenerateMesh()
	{
		mesh.clear();
		int cnt = 0;
		for (float phi = -pi / 2; phi < pi / 2; phi += 0.1)
			for (float alpha = 0.0; alpha < 2 * pi; alpha += 0.1) {
				glm::vec3 currentPos = glm::vec3(0.0f);
				currentPos.x = radius * std::cos(phi) * std::cos(alpha);
				currentPos.y = radius * std::cos(phi) * std::sin(alpha);
				currentPos.z = radius * std::sin(phi);
				Vertex vertex;
				vertex.Position = currentPos;
				vertex.Normal = glm::vec3(std::cos(phi) * std::cos(alpha), std::cos(phi) * std::sin(alpha), std::sin(phi));
				vertex.TexCoords = glm::vec2(alpha / 2.0f / pi, (phi + pi / 2) / pi);
				mesh.vertices.push_back(vertex);
			}
		for (int i = 0; i < mesh.vertices.size() - 64; i++)
		{
			mesh.indices.push_back(i),
				mesh.indices.push_back(i + 1),
				mesh.indices.push_back(i + 63);
			mesh.indices.push_back(i + 63),
				mesh.indices.push_back(i + 64),
				mesh.indices.push_back(i + 1);
		}
		for (int i = 1; i <= 63; i++)
			mesh.indices.push_back(mesh.vertices.size() - 1),
			mesh.indices.push_back(mesh.vertices.size() - i),
			mesh.indices.push_back(mesh.vertices.size() - i - 1);
		mesh.setup();
	}

	void Draw(Shader& shader)
	{
		if (!living)return;
		if (glm::length(V)) {
			//printf("Drawing pos(%lf,%lf,%lf) V(%lf,%lf,%lf)\n", position.x, position.y, position.z, V.x, V.y, V.z);
		}
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		shader.setMat4("model", modelMatrix);
		shader.setInt("type", displayType);
		mesh.Draw(shader);
	}
};

class BallSystem {
public:
	int N = 30;
	const float maxSpeed = 1.0f;
	Ball balls[30];

	bool isActivated = false;

	Texture woodTexture;

	BallSystem() {
	}

	void InitBalls() {
		CreateTexture();
		Randomizer rdm;
		for (int i = 0; i < N; i++) {
			balls[i].initParam(0.0f, 0.0f, 0.0f, 0.03f);
			balls[i].V = glm::vec3(rdm.random(-maxSpeed, maxSpeed), rdm.random(-maxSpeed, maxSpeed), rdm.random(-maxSpeed, maxSpeed));
		//	balls[i].G = glm::vec3(0.0f);
			balls[i].mesh.textures.push_back(woodTexture);
		}
	}

	void Activate() {
		if (isActivated) {
			Randomizer rdm;
			isActivated = false;
			for (int i = 0; i < N; i++) {
				balls[i].initParam(0.0f, 0.0f, 0.0f, 0.03f);
				balls[i].V = glm::vec3(rdm.random(-maxSpeed, maxSpeed), rdm.random(-maxSpeed, maxSpeed), rdm.random(-maxSpeed, maxSpeed));
				balls[i].mesh.textures.push_back(woodTexture);
			}

			balls[0].V = glm::vec3(0.0f), balls[0].G = glm::vec3(0.0f);
			return;
		}
		isActivated = true;
		for (int i = 0; i < N; i++)balls[i].living = true;
	}

	void Animate(float deltaTime) {
		//if (!isActivated)return;
		for (int i = 0; i < N; i++)balls[i].KineticMove(deltaTime);
	}

	void Draw(Shader& shader) {
		shader.use();
		for (int i = 0; i < N; i++)balls[i].Draw(shader);
	}
	void renderShadow(Shader& shader) {
		for (int i = 0; i < N; i++)balls[i].Draw(shader);
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

	void Debug(glm::vec3 raySource, glm::vec3 rayDirection) {
		balls[0].initParam(raySource.x, raySource.y, raySource.z, 0.03f);
		balls[0].mesh.textures.push_back(woodTexture);
		balls[0].V = rayDirection;
		balls[0].G = glm::vec3(0);
		balls[0].living = true;
	}
};


#endif
