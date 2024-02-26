#pragma once
#ifndef FIREANIMATION_H
#define FIREANIMATION_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"
#include "Particle.h"

class FireBall {
public:
	Mesh mesh;
	glm::vec3 V;
	glm::vec3 position;
	float radius = 0.03f;

	bool living = true;
	const float destroyLimit = 4.0f;

	ParticleSystem fireParticles;

	FireBall() { living = false; }

	void Launch(glm::vec3 pos, glm::vec3 Dir) {
		if (living)return;

		const float speed = 1.0f;
		position = pos;
		if(length(Dir))V = Dir / glm::length(Dir) * speed;
		
		GenerateMesh();
		living = true;
		fireParticles.Activate(position, 3.0f, true, V, glm::vec3(249.0f/256, 212.0f/256, 35.0f/256), glm::vec3(1.0f, 78.0f/256, 80.0f/256), 0.013f, 0.8f, 0.18f, 0.0f);
	}

	void Update(float deltaTime) {
		if (!living)return;
		position += V * deltaTime;
		fireParticles.Update(deltaTime);

		if (abs(position.x) >= destroyLimit || abs(position.y) >= destroyLimit || abs(position.z) >= destroyLimit)
			living = false, fireParticles.Deactivate();
	}

	void Draw(Shader& ballShader, Shader& particleShader) {
		if (!living)return;
		ballShader.use();
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		ballShader.setMat4("model", modelMatrix);
		mesh.Draw(ballShader);
		fireParticles.Draw(particleShader);
	}
	void renderShadow(Shader& shader) {
		if (!living)return;
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		shader.setMat4("model", modelMatrix);
		mesh.Draw(shader);
	}

	void GenerateMesh()
	{
		mesh.clear();
		int cnt = 0;
		for (float phi = -glm::pi<float>() / 2; phi < glm::pi<float>() / 2 / 2; phi += 0.1)
			for (float alpha = 0.0; alpha < 2 * glm::pi<float>(); alpha += 0.1) {
				glm::vec3 currentPos = glm::vec3(0.0f);
				currentPos.x = radius * std::cos(phi) * std::cos(alpha);
				currentPos.y = radius * std::cos(phi) * std::sin(alpha);
				currentPos.z = radius * std::sin(phi);
				Vertex vertex;
				vertex.Position = currentPos;
				vertex.Normal = glm::vec3(std::cos(phi) * std::cos(alpha), std::cos(phi) * std::sin(alpha), std::sin(phi));
				vertex.TexCoords = glm::vec2(alpha / 2.0f / glm::pi<float>(), (phi + glm::pi<float>() / 2) / glm::pi<float>());
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
};

class StaticParticleManager {
public:
	std::vector<std::shared_ptr<ParticleSystem>> ps;
	StaticParticleManager(){
		ps.clear();
	}

	void addConicalParticles(glm::vec3 pos, float espeed, glm::vec3 col1, glm::vec3 col2, float pspeed, glm::vec3 norm) {
		std::shared_ptr<ParticleSystem> newPs = std::make_shared<ParticleSystem>();
		ps.push_back(newPs);
		newPs->oriented = true, newPs->orientation = norm;
		newPs->Activate(pos,espeed,false,glm::vec3(0.0f),col1,col2,0.01f,2.0f,pspeed);
	}

	void Update(float deltaTime) {
		for (int i = 0; i < ps.size(); i++) {
			ps[i]->Update(deltaTime);
			if (ps[i]->enabled && ps[i]->particles.size() == 0)ps.erase(ps.begin() + i), i--;
		}
	}
	void Draw(Shader& shader) {
		for (const auto& pss : ps) {
			pss->Draw(shader);
		}
	}

	void SE_Ash(glm::vec3 pos, glm::vec3 norm) {
		addConicalParticles(pos, 30.0f, glm::vec3(0.16171875f), glm::vec3(0.0f), 0.2f, norm);
	}
	void SE_Sparkle(glm::vec3 pos, glm::vec3 norm) {
		addConicalParticles(pos, 200.0f, glm::vec3(249.0f / 256, 212.0f / 256, 35.0f / 256), glm::vec3(248.0f / 256, 54.0f / 256, 0.0f), 0.8f, norm);
	}
};

#endif // !FIREANIMATION_H