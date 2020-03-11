#include "pch.h"
#include "vec3.h"
#include "Boid.h"
#include <iostream>
#include <random>
#include <vector>
#include "time.h"

constexpr int numBoids = 100;
constexpr int numObst = 40;

int main()
{
	srand(time(NULL));

    //Create a set of boids
	std::vector<Boid> boids;
	boids.reserve(numBoids);
	for (int i = 0; i < numBoids; i++)
	{
		vec3 pos = vec3((rand() % 201) - 100, (rand() % 201) - 100, (rand() % 201) - 100);
		vec3 vel = vec3((rand() % 7) - 3, (rand() % 7) - 3, (rand() % 7) - 3);
		boids.emplace_back(Boid(pos, vel, 5.0f, 0.0f, 5.0f, 5.0f));
	}

	//Create a set of obstacles
	std::vector<vec3> obstacles;
	obstacles.reserve(numObst);
	for (int i = 0; i < numObst; i++)
	{
		vec3 pos = vec3((rand() % 201) - 100, (rand() % 201) - 100, (rand() % 201) - 100);
		obstacles.emplace_back(pos);
	}

	//Loop updates until a key is pressed
	clock_t timeCounter = clock();
	while (true)
	{
		for (Boid& boid : boids)
		{
			boid.update(boids, obstacles);
		}
		float deltaT = static_cast<float>((clock() - timeCounter) / CLOCKS_PER_SEC);
		timeCounter = clock();
		for (Boid& boid : boids)
		{
			boid.simulate(deltaT);
		}
	}
}