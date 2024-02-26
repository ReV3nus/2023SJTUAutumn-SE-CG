#pragma once
#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"
#include "Randomizer.h"

class Particle {
public:
	Mesh mesh;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 startColor;
	glm::vec3 endColor;

	float size;
	float sizeAttenuation;
	float lifeTime;
	float livedTime;

	glm::vec3 V;
	glm::vec3 rotateV;
	glm::vec3 G = glm::vec3(0.0f, -0.98f, 0.0f);
	float f_k = 0.1f;
	
	Particle(glm::vec3 pos, glm::vec3 col1, glm::vec3 col2, glm::vec3 speed, glm::vec3 rspeed, float siz, float lifet, glm::vec3 GG=glm::vec3(0.0f,-0.98f,0.0f))
		:position(pos),startColor(col1),endColor(col2),V(speed),rotateV(rspeed),size(siz),lifeTime(lifet),G(GG) {
		sizeAttenuation = size / lifeTime;
		generateMesh();
		livedTime = 0.0f;
	}
	~Particle() {
	}
	void generateMesh() {
		mesh.clear();

		Vertex vertex;
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.Position = glm::vec3(-size, -size, -size);
		mesh.vertices.push_back(vertex);
		vertex.Position = glm::vec3(-size, -size, size);
		mesh.vertices.push_back(vertex);
		vertex.Position = glm::vec3(size, -size, -size);
		mesh.vertices.push_back(vertex);
		vertex.Position = glm::vec3(size, -size, size);
		mesh.vertices.push_back(vertex);
		vertex.Position = glm::vec3(-size, size, -size);
		mesh.vertices.push_back(vertex);
		vertex.Position = glm::vec3(-size, size, size);
		mesh.vertices.push_back(vertex);
		vertex.Position = glm::vec3(size, size, -size);
		mesh.vertices.push_back(vertex);
		vertex.Position = glm::vec3(size, size, size);
		mesh.vertices.push_back(vertex);
		
		//mesh.indices.push_back(0), mesh.indices.push_back(1), mesh.indices.push_back(3);
		//mesh.indices.push_back(0), mesh.indices.push_back(2), mesh.indices.push_back(3);
		//mesh.indices.push_back(4), mesh.indices.push_back(5), mesh.indices.push_back(7);
		//mesh.indices.push_back(4), mesh.indices.push_back(6), mesh.indices.push_back(7);
		//mesh.indices.push_back(0), mesh.indices.push_back(1), mesh.indices.push_back(5);
		//mesh.indices.push_back(0), mesh.indices.push_back(4), mesh.indices.push_back(5);
		//mesh.indices.push_back(1), mesh.indices.push_back(7), mesh.indices.push_back(3);
		//mesh.indices.push_back(1), mesh.indices.push_back(7), mesh.indices.push_back(5);
		//mesh.indices.push_back(2), mesh.indices.push_back(7), mesh.indices.push_back(6);
		//mesh.indices.push_back(2), mesh.indices.push_back(7), mesh.indices.push_back(3);
		//mesh.indices.push_back(0), mesh.indices.push_back(6), mesh.indices.push_back(2);
		//mesh.indices.push_back(0), mesh.indices.push_back(6), mesh.indices.push_back(4);
		mesh.indices.push_back(0), mesh.indices.push_back(5), mesh.indices.push_back(6);
		mesh.indices.push_back(3), mesh.indices.push_back(6), mesh.indices.push_back(5);
		mesh.indices.push_back(3), mesh.indices.push_back(6), mesh.indices.push_back(0);
		mesh.indices.push_back(3), mesh.indices.push_back(5), mesh.indices.push_back(0);
		mesh.setup();
	}
	void updateMesh() {
		if (mesh.vertices.size() < 8)return;
		mesh.vertices[0].Position = glm::vec3(-size, -size, -size);
		mesh.vertices[1].Position = glm::vec3(-size, -size, size);
		mesh.vertices[2].Position = glm::vec3(size, -size, -size);
		mesh.vertices[3].Position = glm::vec3(size, -size, size);
		mesh.vertices[4].Position = glm::vec3(-size, size, -size);
		mesh.vertices[5].Position = glm::vec3(-size, size, size);
		mesh.vertices[6].Position = glm::vec3(size, size, -size);
		mesh.vertices[7].Position = glm::vec3(size, size, size);
		mesh.setup();
	}
	
	bool Calc(float deltaTime) {
		position += V * deltaTime;
		if (position.y < -1.0f)position.y = -1.0f;
		if (position.y > 1.0f)position.y = 1.0f;
		if (position.x < -1.0f)position.x = -1.0f;
		if (position.x > 1.0f)position.x = 1.0f;
		if (position.z < -1.0f)position.z = -1.0f;

		V += (G + glm::length(V) * (-V) * f_k) * deltaTime;
		size -= sizeAttenuation * deltaTime;
		livedTime += deltaTime;
		if (size > 0)updateMesh();
		else return 0;
		return 1;
	}
	void Draw(Shader& shader)
	{
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		shader.setMat4("model", modelMatrix);
		glm::vec3 color = startColor * (lifeTime - livedTime) / lifeTime + endColor * (livedTime / lifeTime);
		shader.setVec3("particleColor", color);
		mesh.Draw(shader);
	}
};
class ParticleSystem {
public:
	std::vector<std::shared_ptr<Particle>>particles;
	
	glm::vec3 centerPos;
	float emitSpeed;
	bool consist;

	glm::vec3 systemV;
	
	glm::vec3 sColor,eColor;
	float particleSize, particleLife, particleSpeed;
	float particleG;

	Randomizer rdm;
	bool enabled = false;
	bool oriented = false;
	glm::vec3 orientation;

	ParticleSystem() {
		particles.clear();
	}

	void Activate(glm::vec3 pos, float espeed, bool consis, glm::vec3 sysV, glm::vec3 col1, glm::vec3 col2, float psize, float plife, float pspeed, float GG = -0.98f){
		centerPos = pos, emitSpeed = espeed, consist = consis, systemV = sysV, sColor = col1, eColor = col2;
		particleSize = psize, particleLife = plife, particleSpeed = pspeed, particleG = GG;
		particles.clear();
		enabled = true;
		generateParticles((int)(emitSpeed));
	}
	void Deactivate() {
		enabled = false;
		particles.clear();
	}
	
	void generateParticles(int num) {
		while (num--) {
			glm::vec3 speed = glm::vec3(rdm.random(-1.0f, 1.0f), rdm.random(-1.0f, 1.0f), rdm.random(-1.0f, 1.0f)) * particleSpeed + systemV;
			if (oriented) {
				if (glm::dot(speed, orientation) < 0)speed = -speed;
			}
			glm::vec3 rspeed = glm::vec3(rdm.random(-1.0f, 1.0f), rdm.random(-1.0f, 1.0f), rdm.random(-1.0f, 1.0f)) * particleSpeed;
			glm::vec3 sysDir = systemV;
			if (glm::length(sysDir))sysDir /= glm::length(sysDir);
			particles.push_back(std::make_shared<Particle>(centerPos, sColor, eColor, speed, rspeed, particleSize, particleLife, -sysDir + glm::vec3(0.0f, particleG, 0.0f)));
		}
	}

	void Update(float deltaTime) {
		if (!enabled)return;
		centerPos += systemV * deltaTime;
		int len = particles.size();
		for (int i = 0; i < len; i++) {
			if (!particles[i]->Calc(deltaTime))
				particles.erase(particles.begin() + i), i--, len--;
		}
		if (consist)generateParticles((int)emitSpeed / deltaTime);
	}

	void Draw(Shader& shader)
	{
		if (!enabled)return;
		shader.use();
		for (const auto& particle : particles)
			particle->Draw(shader);
	}
};



#endif // !PARTICLE_H