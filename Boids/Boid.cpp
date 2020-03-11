#include "pch.h"
#include "Boid.h"
#include <cmath>

//Adds a vector to another, capping the length of the second such that the result's length is 1 or less
void accumulate(vec3& acc, vec3 add)
{
	if (acc.mag() == 1.0f)
		return;

	float dot = acc.dot(add);
	float root = std::sqrtf((dot * dot) - (add.square() * (acc.square() - 1)));
	float A = (-dot + root) / (acc.square() - 1);
	float B = (-dot - root) / (acc.square() - 1);

	if (A >= 0.0f)
	{
		A = std::fminf(1.0f, A);
		acc = acc + (add * A);
		acc = acc.unit();
		return;
	}
	else if (B >= 0.0f)
	{
		B = std::fminf(1.0f, B);
		acc = acc + (add * B);
		acc = acc.unit();
		return;
	}
}

vec3 collisionAvoidance(std::vector<vec3>& obstacles, vec3 pos, float avoidDist)
{
	vec3 sum = vec3();
	for (vec3 obj : obstacles)
	{
		vec3 diff = obj - pos;
		if (diff.square() < avoidDist)
		{
			sum = sum + diff.unit();
		}
	}
	if (sum.mag() > 1)
		return sum.unit();
	else
		return sum;
}

void Boid::update(std::vector<Boid>& boids, std::vector<vec3>& obstacles)
{
	//Find "flock" data
	vec3 sumPos = 0.0f;
	vec3 sumVel = 0.0f;
	for (Boid& boid : boids)
	{
		//rather than creating the flock store the average of nearby velocities and positions simultaneously as it saves on temp data
		if (position =)
	}
	//Clear acceleration accumulator
	acceleration = vec3();
	//Collision avoidance
	vec3 coll = collisionAvoidance(obstacles, position, avoidanceDistance);
	//Match velocity with flock

	//Accumulate
	//Move toward flock centre
	//Accumulate
	//Random acceleration
	//Accumulate

	//Apply acceleration
	//Apply velocity
}

Boid::Boid(vec3 pos, vec3 vel, float acc, float drag, float avoid) : position(pos), velocity(vel), acceleration(vec3(0.0f, 0.0f, 0.0f)), maxAcceleration(acc), dragEffect(drag), avoidanceDistance(avoid)
{
}

Boid::~Boid()
{
}
