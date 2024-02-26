#ifndef SELFUTILS_H
#define SELFUTILS_H

#include "Mesh.h"
#include "FireAnimation.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Ball.h"
#include "Plane.h"
#include "tumbler.h"

const float fireBallMass = 3.0f;

void InitShader(Shader& shader, glm::mat4 projection, glm::mat4 view, glm::vec3 cameraPos) {
	shader.use();
	shader.setVec3("light.position", 0.0f, 1.0f, 0.5f);
	shader.setVec3("viewPos", cameraPos);

	shader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
	shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
	shader.setVec3("light.specular", 0.3f, 0.3f, 0.3f);
	shader.setFloat("light.constant", 1.0f);
	shader.setFloat("light.linear", 0.14f);
	shader.setFloat("light.quadratic", 0.07f);


	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	shader.setMat4("model", glm::mat4(1.0f));

	shader.setBool("isFireballActive", false);
	shader.setVec3("fireballLight.position", 0.0f, 0.0f, -3.6f);
	shader.setVec3("fireballLight.ambient", 248.0f / 256 * 0.0f, 54.0f / 256 * 0.0f, 0.0f);
	shader.setVec3("fireballLight.diffuse", 248.0f/256*0.3f, 54.0f/256*0.3f, 0.0f);
	shader.setVec3("fireballLight.specular", 249.0f/256, 212.0f/256, 35.0f/256);
	shader.setFloat("fireballLight.constant", 1.0f);
	shader.setFloat("fireballLight.linear", 0.14f);
	shader.setFloat("fireballLight.quadratic", 0.07f);
}

void UpdateFireballToShader(Shader& shader, FireBall& fireball) {
	shader.use();
	shader.setBool("isFireballActive", fireball.living);
	shader.setVec3("fireballLight.position", fireball.position);
}

bool Ball_WallCollide(Ball &ball, Room &room) {
	if (ball.position.x < -(room.size + ball.radius) || ball.position.x >(room.size + ball.radius) ||
		ball.position.y < -(room.size + ball.radius) || ball.position.y >(room.size + ball.radius) ||
		ball.position.z < -(room.size + ball.radius) || ball.position.z >(room.size + ball.radius))return false;
	for (int i = 0; i < 4; i++) {
		glm::vec3 pos1 = room.walls[i].mesh.vertices[0].Position;
		glm::vec3 normal = room.walls[i].mesh.vertices[0].Normal;
		float dist = glm::dot((pos1 - ball.position), normal) / glm::length(normal);
		if (glm::abs(dist) <= ball.radius) {
			ball.V = ball.V - 2 * glm::dot(ball.V, normal) * normal;
			ball.displayType = (dType)(Wall0 + i);
			return true;
		}
	}
	glm::vec3 pos1 = room.ground.mesh.vertices[0].Position;
	glm::vec3 normal = room.ground.mesh.vertices[0].Normal;
	float dist = glm::dot((pos1 - ball.position), normal) / glm::length(normal);
	if (glm::abs(dist) <= ball.radius) {
		ball.Reflect(normal);
		ball.displayType = Ground;
		return true;
	}
	return false;
}
bool Ball_TumblerCollide(Ball& ball, Tumbler& tumbler) {
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, tumbler.position);
	glm::vec3 axis = glm::vec3(cos(tumbler.normAngle), 0, -sin(tumbler.normAngle));
	modelMatrix = glm::rotate(modelMatrix, tumbler.axisAngle, axis);

	glm::vec3 bottomBallPos = glm::vec3(modelMatrix * glm::vec4(glm::vec3(0.0f), 1.0));
	glm::vec3 bottomDir = glm::vec3(modelMatrix * glm::vec4(glm::vec3(0.0f, -1.0f, 0.0f), 1.0));
	glm::vec3 topBallPos = glm::vec3(modelMatrix * glm::vec4(glm::vec3(0.0f, 0.15f, 0.0f), 1.0));
	glm::vec3 topDir = glm::vec3(modelMatrix * glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), 1.0));

	float dist = glm::distance(bottomBallPos, ball.position);
	const float FarThreshold = 0.2f;
	if (dist > FarThreshold)return false;

	// check bottom half-ball
	if (dist <= ball.radius + 0.1f &&
		glm::dot((ball.position - bottomBallPos), bottomDir) >= 0) {
		glm::vec3 normal = (ball.position - bottomBallPos);
		normal /= glm::length(normal);
		
		// tumbler kinetic calcs

		// ball kinetic calcs
		ball.Reflect(normal);
		ball.displayType = Tumblers;
		//printf("collide bottom half.\n");
		return true;
	}

	// check top half-ball
	dist = glm::distance(topBallPos, ball.position);
	if (dist <= ball.radius + 0.1f &&
		glm::dot((ball.position - topBallPos), topDir) >= 0) {
		glm::vec3 normal = (ball.position - topBallPos);
		normal /= glm::length(normal);

		// tumbler kinetic calcs
		tumbler.CollideCalculation(topBallPos + (ball.position - topBallPos) * (0.05f / glm::length(ball.position - topBallPos)), ball.V, normal);

		// ball kinetic calcs
		ball.Reflect(normal);
		ball.displayType = Tumblers;
		//printf("collide top half.\n");
		return true;
	}
	
	// check body
	glm::vec3 BodyNorm = topBallPos - bottomBallPos;
	glm::vec3 BallDir = ball.position - bottomBallPos;
	float angle = glm::dot(BallDir, BodyNorm) / glm::length(BallDir) / glm::length(BodyNorm);
	if (angle < 0)return false;
	angle = glm::acos(angle);
	float expectedLength = 0.3f / (glm::cos(angle) + 3*glm::sin(angle));
	if (expectedLength * glm::cos(angle) > 0.15f)return false;
	if (expectedLength + ball.radius >= glm::length(BallDir)) {
		glm::vec3 FaceVector = glm::cross(BodyNorm, BallDir);
		glm::vec3 normal = BodyNorm;
		glm::mat4 rotateMat = glm::mat4(1.0f);
		float rotateAngle = std::atan(3.0f);
		rotateMat = glm::rotate(rotateMat, rotateAngle, FaceVector);
		normal = glm::vec3(rotateMat * glm::vec4(BodyNorm, 1.0));
		normal /= glm::length(normal);
		//printf("BodyNorm (%lf,%lf,%lf)\n", BodyNorm.x, BodyNorm.y, BodyNorm.z);
		//printf("BallDir (%lf,%lf,%lf)\n", BallDir.x, BallDir.y, BallDir.z);
		//printf("FaceVector (%lf,%lf,%lf)\n", FaceVector.x, FaceVector.y, FaceVector.z);
		//printf("angle = %lf\n", rotateAngle);
		//printf("normal (%lf,%lf,%lf)\n", normal.x, normal.y, normal.z);
		// tumbler kinetic calcs
		tumbler.CollideCalculation(bottomBallPos + (expectedLength / glm::length(BallDir)) * BallDir, ball.V, normal);

		// ball kinetic calcs
		ball.Reflect(normal);
		ball.displayType = Tumblers;
		//printf("collide body.\n");
		//ball.living = false;
		return true;
	}

	//Xsina*3=0.3-Xcosa


	return false;
}
void Balls_CollideCalculation(BallSystem &ballSys, Room &room, TumblerCluster &tumblers) {
	if (!ballSys.isActivated)return;
	for (int i = 0; i < ballSys.N; i++) {
		if (!ballSys.balls[i].living)continue;
		bool tumblerFlag = false;
		for (int j = 0; j < 5; j++)
			if (Ball_TumblerCollide(ballSys.balls[i], tumblers.tumblers[j])) {
				tumblerFlag = true;
				break;
			}
		if (tumblerFlag)continue;
		if (Ball_WallCollide(ballSys.balls[i], room))continue;

	}
}

bool Fireball_WallCollide(FireBall &fireBall, Room& room, StaticParticleManager& ptm) {
	if (fireBall.position.x < -(room.size + fireBall.radius) || fireBall.position.x >(room.size + fireBall.radius) ||
		fireBall.position.y < -(room.size + fireBall.radius) || fireBall.position.y >(room.size + fireBall.radius) ||
		fireBall.position.z < -(room.size + fireBall.radius) || fireBall.position.z >(room.size + fireBall.radius))return false;
	for (int i = 0; i < 4; i++) {
		glm::vec3 pos1 = room.walls[i].mesh.vertices[0].Position;
		glm::vec3 normal = room.walls[i].mesh.vertices[0].Normal;
		float dist = glm::dot((pos1 - fireBall.position), normal) / glm::length(normal);
		if (glm::abs(dist) <= fireBall.radius) {
			fireBall.living = false;
			ptm.SE_Sparkle(fireBall.position, normal);
			return true;
		}
	}
	glm::vec3 pos1 = room.ground.mesh.vertices[0].Position;
	glm::vec3 normal = room.ground.mesh.vertices[0].Normal;
	float dist = glm::dot((pos1 - fireBall.position), normal) / glm::length(normal);
	if (glm::abs(dist) <= fireBall.radius) {
		fireBall.living = false;
		ptm.SE_Sparkle(fireBall.position, normal);
		return true;
	}
	return false;
}
bool Fireball_TumblerCollide(FireBall& fireBall, Tumbler& tumbler, StaticParticleManager& ptm) {
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, tumbler.position);
	glm::vec3 axis = glm::vec3(cos(tumbler.normAngle), 0, -sin(tumbler.normAngle));
	modelMatrix = glm::rotate(modelMatrix, tumbler.axisAngle, axis);

	glm::vec3 bottomBallPos = glm::vec3(modelMatrix * glm::vec4(glm::vec3(0.0f), 1.0));
	glm::vec3 bottomDir = glm::vec3(modelMatrix * glm::vec4(glm::vec3(0.0f, -1.0f, 0.0f), 1.0));
	glm::vec3 topBallPos = glm::vec3(modelMatrix * glm::vec4(glm::vec3(0.0f, 0.15f, 0.0f), 1.0));
	glm::vec3 topDir = glm::vec3(modelMatrix * glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), 1.0));

	float dist = glm::distance(bottomBallPos, fireBall.position);
	const float FarThreshold = 0.2f;
	if (dist > FarThreshold)return false;

	// check bottom half-ball
	if (dist <= fireBall.radius + 0.1f &&
		glm::dot((fireBall.position - bottomBallPos), bottomDir) >= 0) {
		glm::vec3 normal = (fireBall.position - bottomBallPos);
		normal /= glm::length(normal);

		// tumbler kinetic calcs

		// ball kinetic calcs
		fireBall.living = false;
		ptm.SE_Sparkle(fireBall.position, normal);
		//printf("collide bottom half.\n");
		return true;
	}

	// check top half-ball
	dist = glm::distance(topBallPos, fireBall.position);
	if (dist <= fireBall.radius + 0.1f &&
		glm::dot((fireBall.position - topBallPos), topDir) >= 0) {
		glm::vec3 normal = (fireBall.position - topBallPos);
		normal /= glm::length(normal);

		// tumbler kinetic calcs
		tumbler.CollideCalculation(topBallPos + (fireBall.position - topBallPos) * (0.05f / glm::length(fireBall.position - topBallPos)), fireBall.V * fireBallMass, normal);

		// ball kinetic calcs
		fireBall.living = false;
		ptm.SE_Sparkle(fireBall.position, normal);
		//printf("collide top half.\n");
		return true;
	}

	// check body
	glm::vec3 BodyNorm = topBallPos - bottomBallPos;
	glm::vec3 BallDir = fireBall.position - bottomBallPos;
	float angle = glm::dot(BallDir, BodyNorm) / glm::length(BallDir) / glm::length(BodyNorm);
	if (angle < 0)return false;
	angle = glm::acos(angle);
	float expectedLength = 0.3f / (glm::cos(angle) + 3 * glm::sin(angle));
	if (expectedLength * glm::cos(angle) > 0.15f)return false;
	if (expectedLength + fireBall.radius >= glm::length(BallDir)) {
		glm::vec3 FaceVector = glm::cross(BodyNorm, BallDir);
		glm::vec3 normal = BodyNorm;
		glm::mat4 rotateMat = glm::mat4(1.0f);
		float rotateAngle = std::atan(3.0f);
		rotateMat = glm::rotate(rotateMat, rotateAngle, FaceVector);
		normal = glm::vec3(rotateMat * glm::vec4(BodyNorm, 1.0));
		normal /= glm::length(normal);
		// tumbler kinetic calcs
		tumbler.CollideCalculation(bottomBallPos + (expectedLength / glm::length(BallDir)) * BallDir, fireBall.V * fireBallMass, normal);

		// ball kinetic calcs
		fireBall.living = false;
		ptm.SE_Sparkle(fireBall.position, normal);
		return true;
	}
	return false;
}
bool Fireball_BallCollide(FireBall& fireBall, Ball &ball, StaticParticleManager& ptm) {
	if (glm::length(ball.position - fireBall.position) > ball.radius + fireBall.radius)return false;
	glm::vec3 normal1 = ball.position - fireBall.position;
	normal1 /= glm::length(normal1);
	ball.living = false;
	fireBall.living = false;
	ptm.SE_Sparkle(fireBall.position, -normal1);
	ptm.SE_Ash(ball.position, normal1);

	return true;
}
void Fireball_CollideCalculation(FireBall& fireball, BallSystem& ballSys, Room& room, TumblerCluster& tumblers, StaticParticleManager& ptm) {
	if (!fireball.living)return;

	bool flag = false;
	if (ballSys.isActivated) {
		for (int i = 0; i < ballSys.N; i++) {
			if (!ballSys.balls[i].living)continue;
			if (Fireball_BallCollide(fireball, ballSys.balls[i], ptm)) {
				flag = true;
				break;
			}
		}
	}
	if (flag)return;

	for (int i = 0; i < 5; i++)
		if (Fireball_TumblerCollide(fireball, tumblers.tumblers[i], ptm))return;
	if (Fireball_WallCollide(fireball, room, ptm))return;
	return;
}

#endif
