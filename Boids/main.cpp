#include "vec3.h"
#include "Boid.h"
#include "Obstacle.h"
#include "Renderer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "SpacePartition.h"

#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <ctime>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#define _USE_MATH_DEFINES
#include <math.h>

static int initialBoids = 100;
static int initialObst = 10;

static int screenWidth = 1350;
static int screenHeight = 900;

static int lmbPressed = 0;
static int rmbPressed = 0;
static double scrollMoved = 0.0f;

void window_size_callback(GLFWwindow* window, int width, int height)
{
	screenHeight = height;
	screenWidth = width;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		lmbPressed++;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		rmbPressed++;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	scrollMoved += yoffset;
}

void fillEntities(int numBoids, int numObst, float obstRadius, 
	std::vector<Boid>& boids, std::vector<Obstacle>& obstacles,
	SpacePartition& spacePartition, VertexArray& vao, IndexBuffer& ib, 
	Texture& actorTex, Texture& rTex, Texture& bTex, Shader& shader)
{
	//Create a set of boids
	boids.reserve(numBoids);
	for (int i = 0; i < numBoids; i++)
	{
		vec3 pos = vec3((float)(rand() % 201) - 100, (float)(rand() % 201) - 100, 0.0f);
		vec3 vel = vec3((float)(rand() % 7) - 3, (float)(rand() % 7) - 3, 0.0f);
		//start pos, start velocity, max acceleration, drag, 
		boids.emplace_back(pos, vel, spacePartition, vao, ib, actorTex, rTex, bTex, shader);
	}

	//Create a set of obstacles
	obstacles.reserve(numObst);
	for (int i = 0; i < numObst; i++)
	{
		vec3 position = vec3((rand() % 201) - 100, (rand() % 201) - 100, 0.0f);
		obstacles.emplace_back(position, obstRadius, spacePartition);
	}
}

void setUpCircle(float obstRadius, std::vector<Boid>& boids, 
	std::vector<Obstacle>& obstacles, SpacePartition& partition)
{
	float angle = 0.0f;
	//Align boids to circle
	for (int i = 0; i < boids.size(); i++)
	{
		vec3 pos = vec3(cos(angle) * 80.0f, sin(angle) * 80.0f, 0.0f);
		vec3 vel = vec3()-pos.unit();
		boids[i].setPosition(pos);
		boids[i].setVelocity(vel);
		boids[i].setHomeDist(1.0f);
		boids[i].setHomeLocation(vec3() - pos);
		angle += (2 * M_PI / boids.size());
	}
	int numObst = obstacles.size();
	obstacles.clear();
	obstacles.reserve(numObst);
	//Align obstacles to inner circle
	for (int i = 0; i < numObst; i++)
	{
		angle += (2 * M_PI / numObst);
		vec3 position = vec3(cos(angle) * 40.0f, sin(angle) * 40.0f, 0.0f);
		obstacles.emplace_back(position, obstRadius, partition);
	}
}

enum class Placement
{
	actor,
	obstacle,
	destination
};

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
		//Declaration of verteces
		float positions[16] = {
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 0.0f, 1.0f
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

		//Compiling shaders and switching openGL over to using them
		Shader shader("Shader.shader");
		shader.bind();

		//Fetching textures
		Texture actorTex("Arrow.png");
		Texture obstTex("Obstacle.png");
		Texture destTex("Home.png");
		Texture rTex("RadiusRed.png");
		Texture bTex("RadiusBlue.png");

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


		//Matrices variables
		//due to column first ordering MVP is multiplied in reverse: P * V * M
		//Order: left, right, bottom, top, near, far
		float orthoHeight = 100.0f;
		float orthoWidth = orthoHeight * screenWidth / screenHeight;
		glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);

		//Create space partitioning
		SpacePartition spacePartition = SpacePartition(48, 48, 10.0f);

		//Setting up boid properties (updated each frame)
		float simSpeed = 1.0f;
		float boidAcc = 0.1f;
		float boidSpeed = 1.0f;
		float boidHome = 60.0f;
		float boidView = 0.5f;
		float boidRadius = 2.0f;
		float boidAvoid = 20.0f;
		float boidDetect = 11.0f;
		float obstRadius = 2.0f;
		bool boidFlocking = true;
		bool boidClearPathUse = false;
		bool updateSettings = true;
		bool drawAvoid = false;
		bool drawDetect = false;
		vec3 destination = vec3();
		Placement placeType = Placement::actor;

		//Create a set of boids and obstacles
		std::vector<Boid> boids;
		std::vector<Obstacle> obstacles;
		fillEntities(initialBoids, initialObst, obstRadius, boids, obstacles, spacePartition,
			vao, ib, actorTex, rTex, bTex, shader);

		//Set callback triggers
		glfwSetWindowSizeCallback(window, window_size_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetScrollCallback(window, scroll_callback);

		//Loop updates until the window is closed
		//clock_t timeCounter = clock();
		while (!glfwWindowShouldClose(window))
		{
			renderer.clear();
			ImGui_ImplGlfwGL3_NewFrame();
			ImGui::Begin("Controls");

			//Key inputs
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
				translation -= glm::vec3(0.0f, 1.0f, 0.0f);
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
				translation += glm::vec3(0.0f, 1.0f, 0.0f);
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				translation += glm::vec3(1.0f, 0.0f, 0.0f);
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				translation -= glm::vec3(1.0f, 0.0f, 0.0f);
			//Mouse click inputs
			if (glfwGetWindowAttrib(window, GLFW_HOVERED) && (lmbPressed != 0 || rmbPressed != 0))
			{
				//Find mouse position worldspace
				//Y is from BL in simulation but TL in screenspace hence inversion
				double mouseX, mouseY;
				glfwGetCursorPos(window, &mouseX, &mouseY);
				mouseX = (mouseX * orthoWidth * 2 / screenWidth) - orthoWidth;
				mouseY = (double)orthoHeight - (mouseY * orthoHeight * 2 / screenHeight);
				vec3 clickPosition((float)mouseX - translation.x, (float)mouseY - translation.y, 0.0f);

				int lmb = lmbPressed;
				lmbPressed = 0;
				int rmb = rmbPressed;
				rmbPressed = 0;
				for (; lmb != 0; lmb--)
				{
					//Things that happen on left mouse press
				}
				for (; rmb != 0; rmb--)
				{
					//Things that happen on right mouse press
					switch (placeType)
					{
					case Placement::actor:
					{
						boids.emplace_back(clickPosition, vec3(), spacePartition,
							vao, ib, actorTex, rTex, bTex, shader);
						Boid& boid = boids[boids.size() - 1];
						boid.setMaxAcceleration(boidAcc);
						boid.setSpeed(boidSpeed);
						boid.setHomeDist(boidHome);
						boid.setViewArc(boidView);
						boid.setRadius(boidRadius);
						boid.setAvoidanceDist(boidAvoid);
						boid.setDetectionDist(boidDetect);
						boid.setClearUsage(boidClearPathUse);
						boid.setFlocking(boidFlocking);
						boid.setHomeLocation(destination);
					}
						break;
					case Placement::obstacle:
						obstacles.emplace_back(clickPosition, obstRadius, spacePartition);
						break;
					case Placement::destination:
						destination = clickPosition;
						break;
					}
				}
			}
			//Mouse scroll input
			if (glfwGetWindowAttrib(window, GLFW_HOVERED) && scrollMoved != 0.0f)
			{
				float zoomAmount = scrollMoved;
				scrollMoved = 0.0f;
				orthoHeight += zoomAmount * 5.0f;
				orthoHeight = std::max(1.0f, orthoHeight);
				orthoWidth = orthoHeight * screenWidth / screenHeight;
			}
			//Imgui inputs
			{
				//static int counter = 0;
				ImGui::SliderFloat("Simulation speed", &simSpeed, 0.0f, 1.0f);
				ImGui::Text("Actor settings");
				ImGui::SliderFloat("Max Acceleration", &boidAcc, 0.01f, 0.2f);
				ImGui::SliderFloat("Max Speed", &boidSpeed, 0.01f, 1.0f);
				ImGui::SliderFloat("Home Bounds", &boidHome, 1.0f, 200.0f);
				ImGui::SliderFloat("View Arc", &boidView, 0.0f, 1.0f);
				ImGui::SliderFloat("Radius", &boidRadius, 0.1f, 20.0f);
				ImGui::SliderFloat("Avoidance Distance", &boidAvoid, 0.0f, 100.0f);
				ImGui::SliderFloat("Detection Distance", &boidDetect, 0.0f, 100.0f);
				ImGui::SliderFloat("Obstacle Radius", &obstRadius, 0.1f, 20.0f);

				ImGui::Checkbox("Use RVO collision avoidance", &boidClearPathUse);
				ImGui::SameLine();
				ImGui::Checkbox("Use flocking behaviour", &boidFlocking);

				ImGui::Checkbox("Draw avoidance", &drawAvoid);
				ImGui::SameLine();
				ImGui::Checkbox("Draw detection", &drawDetect);

				ImGui::Checkbox("Update Actor Settings", &updateSettings);
				
				//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Restart"))
				{
					boids.clear();
					obstacles.clear();
					orthoHeight = 100.0f;
					orthoWidth = orthoHeight * screenWidth / screenHeight;
					fillEntities(initialBoids, initialObst, obstRadius, boids, obstacles, 
						spacePartition, vao, ib, actorTex, rTex, bTex, shader);
					for (Boid& boid : boids)
					{
						boid.setMaxAcceleration(boidAcc);
						boid.setSpeed(boidSpeed);
						boid.setHomeDist(boidHome);
						boid.setViewArc(boidView);
						boid.setRadius(boidRadius);
						boid.setAvoidanceDist(boidAvoid);
						boid.setDetectionDist(boidDetect);
						boid.setClearUsage(boidClearPathUse);
						boid.setFlocking(boidFlocking);
						boid.setHomeLocation(destination);
					}
				}
				ImGui::SameLine();
				ImGui::InputInt2("", &initialBoids);
				if (ImGui::Button("Circle Test"))
				{
					updateSettings = false;
					setUpCircle(obstRadius, boids, obstacles, spacePartition);
				}

				if (ImGui::Button("Place actor"))
					placeType = Placement::actor;
				ImGui::SameLine();
				if (ImGui::Button("Place obstacle"))
					placeType = Placement::obstacle;
				ImGui::SameLine();
				if (ImGui::Button("Place destination"))
					placeType = Placement::destination;
				switch (placeType)
				{
				case Placement::actor:
					ImGui::Text("RMB places an actor");
					break;
				case Placement::obstacle:
					ImGui::Text("RMB places an obstacle");
					break;
				case Placement::destination:
					ImGui::Text("RMB places the destination");
					break;
				}
				ImGui::Text("Scroll wheel zooms and arrow keys pan camera");
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}

			//Start of rendering and simulation
			glm::mat4 view = glm::translate(glm::mat4(1.0f), translation);
			glm::mat4 projection = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -10.0f, 10.0f);
			glm::mat4 viewProjection = projection * view;

			//Draw current destination marker
			{
				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(destination.x, destination.y, destination.z));
				glm::mat4 modelViewProjection = viewProjection * model;

				shader.bind();
				destTex.bind(0);
				shader.setUniform1i("u_texture", 0);
				shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);

				renderer.draw(vao, ib, shader);
			}

			for (Boid& boid : boids)
			{
				//Allow for in flight adjustments
				if (updateSettings)
				{
					boid.setMaxAcceleration(boidAcc);
					boid.setSpeed(boidSpeed);
					boid.setHomeDist(boidHome);
					boid.setViewArc(boidView);
					boid.setRadius(boidRadius);
					boid.setAvoidanceDist(boidAvoid);
					boid.setDetectionDist(boidDetect);
					boid.setClearUsage(boidClearPathUse);
					boid.setFlocking(boidFlocking);
					boid.setHomeLocation(destination);
				}
				//Run boid steering
				boid.steering();
				//Draw radii
				boid.drawAuras(renderer, viewProjection, drawAvoid, drawDetect);
			}
			
			for (Boid& boid : boids)
			{
				boid.locomotion(simSpeed);
				boid.draw(renderer, viewProjection);
			}

			for (Obstacle& obst : obstacles)
			{
				if (updateSettings)
					obst.m_radius = obstRadius;

				vec3 pos = obst.m_position;
				glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(boidRadius / 2));
				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
				glm::mat4 modelViewProjection = viewProjection * model * scale;

				shader.bind();
				obstTex.bind(0);
				shader.setUniform1i("u_texture", 0);
				shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);

				renderer.draw(vao, ib, shader);
			}
			ImGui::End();
			//Rendering GUI
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