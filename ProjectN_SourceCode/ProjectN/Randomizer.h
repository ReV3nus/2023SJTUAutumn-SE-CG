#pragma once
#ifndef RANDOMIZER_H
#define RANDOMIZER_H

#include<random>
class Randomizer {
public:
	std::random_device rd;

	Randomizer() {}

	float random(float l = 0.0f, float r = 1.0f) {
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(l, r);
		return dis(gen);
	}
};
#endif