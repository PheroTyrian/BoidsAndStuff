#include "vec3.h"
#include "Boid.h"
#include <iostream>
#include <random>
#include <vector>
#include "time.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

constexpr int numBoids = 100;
constexpr int numObst = 40;

int main()
{
	//Basic setup
	srand(time(NULL));
	
	// Initialize the library
	if (!glfwInit())
		return -1;
	// Create a windowed mode window and its OpenGL context
	GLFWwindow* window;
	window = glfwCreateWindow(640, 480, "Boids", NULL, NULL);
	if (!window)
	{
		std::cout << "Oof no window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//Make the window's context current
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

    //Create a set of boids
	std::vector<Boid> boids;
	boids.reserve(numBoids);
	for (int i = 0; i < numBoids; i++)
	{
		vec3 pos = vec3((rand() % 201) - 100, (rand() % 201) - 100, (rand() % 201) - 100);
		vec3 vel = vec3((rand() % 7) - 3, (rand() % 7) - 3, (rand() % 7) - 3);
		boids.emplace_back(pos, vel, 5.0f, 0.0f, 5.0f, 5.0f);
	}

	//Create a set of obstacles
	std::vector<vec3> obstacles;
	obstacles.reserve(numObst);
	for (int i = 0; i < numObst; i++)
	{
		obstacles.emplace_back((rand() % 201) - 100, (rand() % 201) - 100, (rand() % 201) - 100);
	}

	float positions[6] = {
		-0.5f, -0.5f,
		0.0f, 0.5f,
		0.5f, -0.5f,
	};
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

	//Loop updates until the window is closed
	clock_t timeCounter = clock();
	while (!glfwWindowShouldClose(window))
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

		// render
		// ------
		//glClearColor(0.2f, 0.6f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}