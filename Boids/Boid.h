#pragma once
#include "vec3.h"
#include <vector>

class Boid
{
private:
	vec3 position;
	vec3 velocity;
	vec3 acceleration;
	const float maxAcceleration;
	const float dragEffect;
	const float avoidanceDistance;
	const float detectionDistance;

public:
	void update(std::vector<Boid>& boids, std::vector<vec3>& obstacle);
	void simulate(float deltaT);

	Boid();
	Boid(vec3 pos, vec3 vel, float acc, float drag, float avoid, float detect);
	~Boid();
};

