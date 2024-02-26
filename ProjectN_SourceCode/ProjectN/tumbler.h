#ifndef TUMBLER_H
#define TUMBLER_H

#include "Mesh.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<random>

class Tumbler {
public:
	Model *model;

	// Model Params
	glm::vec3 position, scale;
	float normAngle, axisAngle, selfAngle;

	// Kinetic Params
	const float mass = 1.0f, J = 0.3f;
	const float RotationThreshold = 0.05f;
	float selfRotate_v, selfRotate_EnergyLoss = 0.01*J;
	float normRotate_v, normRotate_EnergyLoss = 5*J;
	const float self_param = 10.0f;
	const float norm_param = 8.0f;
	const float axis_param = 1.0f;

	float axisRotate_v, axisRotate_a;
	const float axisRotateSlowrate=1.25f, axisRotate_EnergyLoss = 0.005 * J, axisRotate_KM = 10.0f;

	// Captrue Params
	float total_x_offset, total_y_offset;
	bool beingCaptured = false;

	Tumbler(){
	}
	~Tumbler() {
		delete model;
	}

	void Init() {
		model = new Model("./models/tumbler.obj");
		//modelMatrix = glm::mat4(1.0f);
		position = glm::vec3(0.0f);
		ClearStatus();
		scale = glm::vec3(2.0f);
	}
	void Draw(Shader& shader) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position); // translate it down so it's at the center of the scene
		glm::vec3 axis = glm::vec3(cos(normAngle), 0, -sin(normAngle));
		modelMatrix = glm::rotate(modelMatrix, axisAngle, axis);
		modelMatrix = glm::rotate(modelMatrix, selfAngle, glm::vec3(0.0f, 1.0f, 0.0f));

		modelMatrix = glm::scale(modelMatrix, scale);	// it's a bit too big for our scene, so scale it down
		shader.setMat4("model", modelMatrix);
		model->Draw(shader);
	}

	int isRayDetect(glm::vec3 raySource, glm::vec3 rayDirection) {
		//printf("POS(%lf,%lf,%lf)\n", position.x, position.y, position.z);
		glm::vec3 pos = position;
		pos.y += 0.1f;
		float distance = glm::length(glm::cross((pos - raySource), rayDirection)) / glm::length(rayDirection);
		//printf("DIST1:%lf\n", distance);
		if (distance <= 0.12f)return 1;
		pos.y -= 0.1f;
		distance = glm::length(glm::cross((pos - raySource), rayDirection)) / glm::length(rayDirection);
		//printf("DIST2:%lf\n", distance);
		if (distance <= 0.1f)return 2;
		return 0;
	}

	void KineticCalculation(float deltaTime) {
		if (beingCaptured)return;

		// calc self rotation
		selfAngle += selfRotate_v * deltaTime;

		float E1 = 1.0f / 2 * J * selfRotate_v * selfRotate_v;
		float E2 = max(0.0f, E1 - selfRotate_EnergyLoss * deltaTime);
		selfRotate_v = (selfRotate_v > 0 ? 1.0f : - 1.0f) * glm::sqrt(E2 * 2.0f / J);


		// calc normal axis rotation
		normAngle += normRotate_v * deltaTime;

		const float normJ = J + mass * glm::pow((0.1f * glm::sin(axisAngle)), 2);
		E1 = 1.0f / 2 * normJ * normRotate_v * normRotate_v;
		E2 = max(0.0f, E1 - normRotate_EnergyLoss * deltaTime);
		normRotate_v = (normRotate_v > 0 ? 1.0f : -1.0f) * glm::sqrt(E2 * 2.0f / normJ);

		// calc axis pendulum
		axisAngle += axisRotate_v * deltaTime;
		axisRotate_v += axisRotate_a * deltaTime;
		axisRotate_a = -axisAngle * axisRotate_KM;
		if (axisRotate_v > 0 == axisRotate_a > 0) 
			axisRotate_a /= axisRotateSlowrate;
		else
			axisRotate_a *= axisRotateSlowrate;
		const float axisJ = J + mass * 0.1;
		E1 = 1.0f / 2 * axisJ * axisRotate_v * axisRotate_v + 1.0 / 2 * axisRotate_KM * mass * axisAngle * axisAngle;
		//E2 = E1 - axisRotate_EnergyLoss * deltaTime - (axisRotate_KM * mass) * (1.0f / 2 * axisRotate_v * axisRotate_v * deltaTime * deltaTime + 1.0f / 2 * axisAngle * axisAngle + axisRotate_v * deltaTime * axisAngle);
		//E2 = max(0.0f, E2);
		//axisRotate_v = (axisRotate_v > 0 ? 1.0f : -1.0f) * glm::sqrt(E2 * 2.0f / normJ);

		if (E1 < axisRotate_EnergyLoss) {
			axisAngle = axisRotate_v = axisRotate_a = 0.0f;
		}
		//if (axisAngle || axisRotate_v || axisRotate_a)
		//	printf("angle=%lf v=%lf a=%lf\n", axisAngle, axisRotate_v, axisRotate_a);

		//const float axisJ = J + mass * 0.01;
		//E1 = 1.0f / 2 * axisJ * axisRotate_v * axisRotate_v;
		//E2 = max(0.0f, E1 - axisRotate_EnergyLoss * deltaTime);
		//axisRotate_v = (axisRotate_v > 0 ? 1.0f : -1.0f) * glm::sqrt(E2 * 2.0f / normJ);

	}

	void CollideCalculation(glm::vec3 pos, glm::vec3 ballV, glm::vec3 normal) {
		pos -= position;
		printf("\n-----------------------\nCollide\n");
		printf("pos (%lf,%lf,%lf)\n", pos.x, pos.y, pos.z);
		printf("ballV is (%lf,%lf,%lf)\n", ballV.x, ballV.y, ballV.z);
		printf("normal is (%lf,%lf,%lf)\n", normal.x, normal.y, normal.z);
		glm::vec3 F = 2 * glm::dot(ballV, normal) * normal;
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		glm::vec3 axis = glm::vec3(cos(normAngle), 0, -sin(normAngle));
		modelMatrix = glm::rotate(modelMatrix, axisAngle, axis);
		glm::vec3 SymmAxis = glm::vec3(modelMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		glm::vec3 normAxis = glm::vec3(cos(normAngle), 0, -sin(normAngle));
		printf("SymmAxis (%lf,%lf,%lf)\n", SymmAxis.x, SymmAxis.y, SymmAxis.z);

		glm::vec3 trueF = F - glm::dot(F, SymmAxis) * SymmAxis;
	
		// calc self
		glm::vec3 paralV = ballV - glm::dot(ballV, normal) * normal;
		float self_dist = glm::abs(glm::dot(pos,SymmAxis)/glm::length(SymmAxis));
		float self_ballv = glm::length(glm::cross(SymmAxis, paralV)) / glm::length(SymmAxis);
		if (glm::dot(SymmAxis, glm::cross(pos, paralV)) > 0)
			selfRotate_v += self_dist * self_ballv * self_param;
		else selfRotate_v -= self_dist * self_ballv * self_param;

		//calc norm rotation
		glm::vec3 axis_to_point = glm::vec3(pos.x, 0, pos.z);
		if (glm::dot(axis_to_point, F) <= 0)
			normRotate_v += glm::length(glm::cross(axis_to_point, F)) * norm_param;
		else normRotate_v -= glm::length(glm::cross(axis_to_point, F)) * norm_param;

		//calc axis pendulum
		float origin_angle = normAngle - glm::pi<float>() / 2;
		if (axisAngle < 0)origin_angle += glm::pi<float>();
		const float axisJ = J + mass * 0.1;
		float origin_A = 1.0f / 2 * axisJ * axisRotate_v * axisRotate_v + 1.0f / 2 * axisRotate_KM * mass * axisAngle * axisAngle;
		glm::vec3 TransF = glm::vec3(glm::inverse(modelMatrix) * glm::vec4(F, 1.0f));
		printf("originF is (%lf,%lf,%lf)\n", F.x, F.y, F.z);
		printf("TransF is (%lf,%lf,%lf)\n", TransF.x, TransF.y, TransF.z);
		float Trend_angle = glm::atan((-TransF.z) / TransF.x);
		if (TransF.x < 0)Trend_angle += glm::pi<float>();
		float Trend_A = axis_param * glm::length(glm::vec3(TransF.x, 0.0f, TransF.z));
		glm::vec2 newA = glm::vec2(origin_A * glm::cos(origin_angle) + Trend_A * glm::cos(Trend_angle), origin_A * glm::sin(origin_angle) + Trend_A * glm::sin(Trend_angle));

		printf("origin_angle is %lf\norigin_A is %lf\n", origin_angle / glm::pi<float>() * 180, origin_A);
		printf("trend_angle is %lf\ntrend_A is %lf\n", Trend_angle / glm::pi<float>() * 180, Trend_A);
		printf("newA is (%lf,%lf)\n", newA.x, newA.y);

		float newEnergy = min(glm::length(newA), 1.0f / 2 * axisRotate_KM * mass * glm::pi<float>() * 75 / 180 * glm::pi<float>() * 75 / 180);
		float newV2 = (newEnergy - 1.0f / 2 * axisRotate_KM * mass * axisAngle * axisAngle) * (2.0f / axisJ);
		if (newV2 < 0)newV2 = 0;
		float newV = glm::sqrt(newV2);
		newV = min(newV, (glm::pi<float>() * 75 / 180 - glm::abs(axisAngle)) * 2.0f);
		axisRotate_v = newV;
		if (glm::dot(glm::vec2(cos(origin_angle), sin(origin_angle)), newA) < 0)axisRotate_v = -newV;

		float tmpAngle = atan(newA.y / newA.x);
		if (newA.x < 0)tmpAngle += glm::pi<float>();
		float newNormAngle = tmpAngle + glm::pi<float>() / 2;
		normRotate_v += (newNormAngle - normAngle) / 1.5f;

		printf("normAngle is %lf, new norm angle should be %lf\naxisRotate_v is %lf\n", normAngle/glm::pi<float>()*180, newNormAngle/glm::pi<float>()*180,axisRotate_v);
		printf("normRotate_v = %lf\n", normRotate_v);
	}

	void ClearStatus() {
		normAngle = 0.0f;
		axisAngle = 0.0f;
		selfAngle = 0.0f;
		total_x_offset = 0.0f;
		total_y_offset = 0.0f;
		selfRotate_v = normRotate_v = axisRotate_v = 0.0f;
		axisRotate_a = 0.0f;
	}
	void Tilt_Offset(float xoff, float yoff) {
		float threshold = 1500.0f;
		total_x_offset += xoff;
		total_y_offset += yoff;
		if (sqrt(total_x_offset * total_x_offset + total_y_offset * total_y_offset) >= 1500.0f)
		{
			float tmp = sqrt(total_x_offset * total_x_offset + total_y_offset * total_y_offset) / 1500.0f;
			total_x_offset /= tmp;
			total_y_offset /= tmp;
		}

		//printf("XOFF %lf (+%lf) YOFF %lf (+%lf)\n", total_x_offset, xoff, total_y_offset, yoff);
		if (total_x_offset == 0.0f)total_x_offset = 0.00001f;
		float tmpAngle = atan(total_y_offset / total_x_offset);
		if (total_x_offset < 0)tmpAngle += glm::pi<float>();
		normAngle = tmpAngle + glm::pi<float>() / 2;
		float tmpAmplitude = sqrt(total_x_offset * total_x_offset + total_y_offset * total_y_offset) / 1500.0f;
		axisAngle = glm::pi<float>()*75/180 * tmpAmplitude;

		//printf("normAngle=%lf axisAngle=%lf\n",normAngle,axisAngle);
	}
private:

};

class TumblerCluster {
public:
	Tumbler tumblers[5];
	float groundY = -0.9f;

	int capturedIdx = -1;
	bool isTiltMode = false;

	TumblerCluster() {
	}
	void Init() {
		for (int i = 0; i < 5; i++)tumblers[i].Init();
		tumblers[0].position = glm::vec3(0.0f, groundY, 0.0f);
		tumblers[1].position = glm::vec3(0.5f, groundY, 0.5f);
		tumblers[2].position = glm::vec3(0.5f, groundY, -0.5f);
		tumblers[3].position = glm::vec3(-0.5f, groundY, 0.5f);
		tumblers[4].position = glm::vec3(-0.5f, groundY, -0.5f);
	}
	void Draw(Shader& shader) {
		shader.use();
		for (int i = 0; i < 5; i++) {
			tumblers[i].Draw(shader);
		}
	}
	void renderShadow(Shader& simpleDepthShader) {
		for (int i = 0; i < 5; i++) {
			tumblers[i].Draw(simpleDepthShader);
		}
	}
	
	int checkRay(glm::vec3 raySource, glm::vec3 rayDirection) {
		for (int i = 0; i < 5; i++) {
			int retval = tumblers[i].isRayDetect(raySource, rayDirection);
			if (retval) {
				capturedIdx = i;
				isTiltMode = (retval == 1);
				tumblers[i].ClearStatus();
				tumblers[i].beingCaptured = true;

				//tumblers[i].normRotate_v = 10.0f;
				//tumblers[i].selfRotate_v = 10.0f;
				return i;
			}
		}
		return -1;
	}
	void KineticCalculation(float deltaTime) {
		for (int i = 0; i < 5; i++)
			tumblers[i].KineticCalculation(deltaTime);
	}

	bool CheckPos(float x, float y, int idx) {
		float margin = 0.2f;
		if (x - margin < -1.0f || x + margin > 1.0f || y - margin < -1.0f || y + margin > 1.0f)
			return false;
		for (int i = 0; i < 5; i++) {
			if (i == idx)continue;
			float vx = tumblers[i].position.x, vy = -tumblers[i].position.z;
			if (glm::sqrt((x - vx) * (x - vx) + (y - vy) * (y - vy)) <= 2 * margin)
				return false;
		}
		return true;
	}
	void processOffsets(float xoffset, float yoffset) {
		if (capturedIdx == -1)return;
		if (isTiltMode) {
			tumblers[capturedIdx].Tilt_Offset(xoffset, yoffset);
		}
		else {
			xoffset /= 1500.0f;
			yoffset /= 1500.0f;
			xoffset = CheckPos(tumblers[capturedIdx].position.x + xoffset, -tumblers[capturedIdx].position.z, capturedIdx) ? xoffset : 0;
			yoffset = CheckPos(tumblers[capturedIdx].position.x, -tumblers[capturedIdx].position.z + yoffset, capturedIdx) ? yoffset : 0;
			tumblers[capturedIdx].position.x += xoffset;
			tumblers[capturedIdx].position.z -= yoffset;
		}
	}
	void ReleaseMouse() {
		tumblers[capturedIdx].beingCaptured = false;
		capturedIdx = -1;

	}
};
#endif