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
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

constexpr int screenWidth = 1200;
constexpr int screenHeight = 800;

constexpr int numBoids = 100;
constexpr int numObst = 10;

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

	{
		//Temp declaration of verteces
		float positions[16] = {
			-2.0f, -2.0f, 0.0f, 0.0f,
			2.0f, -2.0f, 1.0f, 0.0f,
			2.0f, 2.0f, 1.0f, 1.0f,
			-2.0f, 2.0f, 0.0f, 1.0f
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
		glm::mat4 projection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, -100.0f, 100.0f);

		//Compiling shaders and switching openGL over to using them
		Shader shader("Shader.shader");
		shader.bind();

		//Fetching textures
		Texture actorTex("Arrow.png");
		Texture obstTex("Obstacle.png");

		//shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);

		//Unbinding everything
		vao.unbind();
		vb.unbind();
		ib.unbind();
		shader.unbind();

		Renderer renderer;

		ImGui::CreateContext();
		ImGui_ImplGlfwGL3_Init(window, true);
		ImGui::StyleColorsDark();

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);

		//Create a set of boids
		std::vector<Boid> boids;
		boids.reserve(numBoids);
		for (int i = 0; i < numBoids; i++)
		{
			vec3 pos = vec3((float)(rand() % 201) - 100, (float)(rand() % 201) - 100, 0.0f);
			vec3 vel = vec3((float)(rand() % 7) - 3, (float)(rand() % 7) - 3, 0.0f);
			//start pos, start velocity, max acceleration, drag, 
			boids.emplace_back(pos, vel, vao, ib, actorTex, shader);
		}

		//Create a set of obstacles
		std::vector<vec3> obstacles;
		obstacles.reserve(numObst);
		for (int i = 0; i < numObst; i++)
		{
			obstacles.emplace_back((rand() % 201) - 100, (rand() % 201) - 100, 0.0f);
		}

		//Setting boid properties (updated each frame)
		float deltaT = 1.0f;
		float boidAcc = 0.1f;
		float boidSpeed = 1.0f;
		float boidHome = 60.0f;
		float boidRadius = 2.0f;
		float boidAvoid = 5.0f;
		float boidDetect = 10.0f;
		bool boidDamping = true;

		//Loop updates until the window is closed
		clock_t timeCounter = clock();
		while (!glfwWindowShouldClose(window))
		{
			renderer.clear();
			ImGui_ImplGlfwGL3_NewFrame();

			glm::mat4 view = glm::translate(glm::mat4(1.0f), translation);
			glm::mat4 viewProjection = projection * view;

			for (Boid& boid : boids)
			{
				//Allow for in flight adjustments
				boid.setMaxAcceleration(boidAcc);
				boid.setSpeed(boidSpeed);
				boid.setHomeDist(boidHome);
				boid.setRadius(boidRadius);
				boid.setAvoidanceDist(boidAvoid);
				boid.setDetectionDist(boidDetect);
				boid.setDamping(boidDamping);
				//Run boid systems
				boid.steering(boids, obstacles);
			}
			//float deltaT = static_cast<float>((clock() - timeCounter) / CLOCKS_PER_SEC);
			timeCounter = clock();
			for (Boid& boid : boids)
			{
				boid.locomotion(deltaT);
				boid.draw(renderer, viewProjection);
			}

			for (vec3 obst : obstacles)
			{
				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(obst.x, obst.y, obst.z));
				glm::mat4 modelViewProjection = viewProjection * model;

				obstTex.bind(0);
				shader.setUniform1i("u_texture", 0);
				shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);

				renderer.draw(vao, ib, shader);
			}
			
			// render
			/*glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
			glm::mat4 modelViewProjection = viewProjection * model;

			shader.bind();
			texture.bind(0);
			shader.setUniform1i("u_texture", 0);
			shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);

			renderer.draw(vao, ib, shader);*/

			//Imgui
			{
				//static int counter = 0;
				ImGui::SliderFloat("Simulation speed", &deltaT, 0.0f, 1.0f);
				ImGui::Text("Boid settings");
				ImGui::SliderFloat("Max Acceleration", &boidAcc, 0.01f, 1.0f);
				ImGui::SliderFloat("Max Speed", &boidSpeed, 0.01f, 1.0f);
				ImGui::SliderFloat("Home Bounds", &boidHome, 1.0f, 200.0f);
				ImGui::SliderFloat("Object Radius", &boidRadius, 0.1f, 20.0f);
				ImGui::SliderFloat("Avoidance Distance", &boidAvoid, 0.0f, 100.0f);
				ImGui::SliderFloat("Detection Distance", &boidDetect, 0.0f, 100.0f);
				ImGui::Checkbox("Damping", &boidDamping);
				//ImGui::SliderFloat("Z", &translation.z, 100.0f, -100.0f);
				//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
				//ImGui::Checkbox("Another Window", &show_another_window);

				//if (ImGui::Button("Counter"))
				//	counter++;
				//ImGui::SameLine();
				//ImGui::Text("= %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}

			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	return 0;
}