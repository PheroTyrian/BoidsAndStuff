#include "pch.h"
#include "Boid.h"
#include <cmath>
#include <random>

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
	vec3 sumPos = vec3();
	vec3 sumVel = vec3();
	for (Boid& boid : boids)
	{
		//rather than creating the flock store the average of nearby velocities and positions simultaneously as it saves on temp data
		vec3 diff = position - boid.position;
		if (diff.square() < detectionDistance)
		{
			if (diff == vec3())
				continue;

			sumPos += boid.position;
			sumVel += boid.velocity;
		}
	}
	sumPos = sumPos.unit();

	//Collision avoidance
	vec3 collision = collisionAvoidance(obstacles, position, avoidanceDistance);

	//Match velocity with flock
	sumVel = sumVel.unit();
	vec3 matchVel = (sumVel - velocity) / maxAcceleration;

	//Move toward flock centre
	sumPos = sumPos.unit();
	vec3 matchPos = (sumPos - position) / detectionDistance;

	//Random acceleration
	vec3 randmotion = vec3(rand() % 21 - 10, rand() % 21 - 10, rand() % 21 - 10);
	randmotion = randmotion.unit();

	//Accumulate
	acceleration = collision;
	accumulate(acceleration, matchVel);
	accumulate(acceleration, matchPos);
	accumulate(acceleration, randmotion);
}

void Boid::simulate(float deltaT)
{
	velocity += acceleration * maxAcceleration * deltaT;
	//Temp code capping velocity in place of drag
	float magnitude = std::min(velocity.mag(), 5.0f);
	velocity = velocity.unit() * magnitude;

	position += velocity * deltaT;
	//Temp code wrapping positions
	if (position.x > 100.0f)
		position.x -= 200.0f;
	if (position.y > 100.0f)
		position.y -= 200.0f;
	if (position.z > 100.0f)
		position.z -= 200.0f;

	if (position.x < -100.0f)
		position.x += 200.0f;
	if (position.y < -100.0f)
		position.y += 200.0f;
	if (position.z < -100.0f)
		position.z += 200.0f;
}

Boid::Boid()
	: position(vec3()), velocity(vec3()), acceleration(vec3()), maxAcceleration(5.0f), dragEffect(0.0f), avoidanceDistance(5.0f), detectionDistance(5.0f)
{
}

Boid::Boid(vec3 pos, vec3 vel, float acc, float drag, float avoid, float detect)
	: position(pos), velocity(vel), acceleration(vec3()), maxAcceleration(acc), dragEffect(drag), avoidanceDistance(avoid), detectionDistance(detect)
{
}

Boid::~Boid()
{
}
