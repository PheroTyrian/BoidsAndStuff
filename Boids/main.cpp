#include "vec3.h"
#include "Boid.h"
#include "Renderer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <ctime>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

constexpr int screenWidth = 640;
constexpr int screenHeight = 480;

constexpr int numBoids = 100;
constexpr int numObst = 40;

int main()
{
	//Basic setup
	std::srand(std::time(NULL));
	
	// Initialize the library
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a windowed mode window and its OpenGL context
	GLFWwindow* window;
	window = glfwCreateWindow(screenWidth, screenHeight, "Boids", NULL, NULL);
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

	{
		//Temp declaration of verteces
		float positions[16] = {
			50.0f, 50.0f, 0.0f, 0.0f,
			400.0f, 50.0f, 1.0f, 0.0f,
			400.0f, 400.0f, 1.0f, 1.0f,
			50.0f, 400.0f, 0.0f, 1.0f
		};

		unsigned int indices[6] = {
			0, 1, 2,
			2, 3, 0
		};
		
		//Enable blending
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLCall(glEnable(GL_BLEND));

		//Binding vertex array object to vertex buffer
		VertexArray vao;
		VertexBuffer vb(positions, 4 * 4 * sizeof(float));
		VertexBufferLayout layout;
		layout.push<float>(2);
		layout.push<float>(2);
		vao.addBuffer(vb, layout);

		IndexBuffer ib(indices, 6);

		//Matrices
		//due to column first ordering MVP is multiplied in reverse: P * V * M
		//Order: left, right, bottom, top, near, far
		glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -1.0f, 1.0f);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 0.0f));

		glm::mat4 mvp = projection * view;

		//Compiling shaders and switching openGL over to using them
		Shader shader("Shader.shader");
		shader.bind();

		//Fetching a texture
		Texture texture("Catpiler.png");
		texture.bind(0);

		//Uniforms
		shader.setUniform1i("u_texture", 0);
		shader.setUniformMat4f("u_modelViewProjection", mvp);
		shader.setUniform4f("u_colour", 0.2f, 0.3f, 0.4f, 1.0f);

		//Unbinding everything
		vao.unbind();
		vb.unbind();
		ib.unbind();
		shader.unbind();
		texture.unbind();

		Renderer renderer;

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
			renderer.clear();

			shader.bind();
			shader.setUniform4f("u_colour", 0.2f, 0.3f, 0.4f, 1.0f);
			texture.bind(0);
			shader.setUniform1i("u_texture", 0);
			shader.setUniformMat4f("u_modelVieWProjection", projection);

			renderer.draw(vao, ib, shader);

			// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	glfwTerminate();
}