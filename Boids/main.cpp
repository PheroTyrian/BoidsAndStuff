#include "vec3.h"
#include "Boid.h"
#include <iostream>
#include <random>
#include <vector>
#include <ctime>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

constexpr int numBoids = 100;
constexpr int numObst = 40;

static int compileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);
	
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length*sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Shader compile error:" << std::endl << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static int createShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	//Delete shader intermediaries
	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main()
{
	//Basic setup
	std::srand(std::time(NULL));
	
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

	std::string vertexShader =
		"#version 330 core \n"
		"layout(location = 0) in vec4 position;\n"
		"void main()\n"
		"{\n"
		"gl_Position = position;\n"
		"}\n";
	std::string fragmentShader =
		"#version 330 core \n"
		"layout(location = 0) out vec4 colour;\n"
		"void main()\n"
		"{\n"
		"colour = vec4(1.0, 1.0, 0.0, 1.0);\n"
		"}\n";
	unsigned int shader = createShader(vertexShader, fragmentShader);
	glUseProgram(shader);

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