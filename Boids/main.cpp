#include "vec3.h"
#include "Boid.h"
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

constexpr int initialBoids = 100;
constexpr int initialObst = 10;

static int screenWidth = 1200;
static int screenHeight = 800;

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

void fillEntities(int numBoids, int numObst, std::vector<Boid>& boids, std::vector<vec3>& obstacles,
	SpacePartition& spacePartition, VertexArray& vao, IndexBuffer& ib, Texture& actorTex, Texture& rTex, Texture& bTex, Shader& shader)
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
		obstacles.emplace_back((rand() % 201) - 100, (rand() % 201) - 100, 0.0f);
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
		float orthoWidth = 100.0f;
		float orthoHeight = 100.0f;
		glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);

		//Create space partitioning
		SpacePartition spacePartition = SpacePartition(32, 32, 5.0f);

		//Create a set of boids and obstacles
		std::vector<Boid> boids;
		std::vector<vec3> obstacles;
		fillEntities(initialBoids, initialObst, boids, obstacles, spacePartition, 
			vao, ib, actorTex, rTex, bTex, shader);

		//Setting boid properties (updated each frame)
		float simSpeed = 1.0f;
		float boidAcc = 0.1f;
		float boidSpeed = 1.0f;
		float boidHome = 60.0f;
		float boidView = 0.5f;
		float boidRadius = 2.0f;
		float boidAvoid = 10.0f;
		float boidDetect = 11.0f;
		bool boidDamping = true;
		bool drawAvoid = false;
		bool drawDetect = false;
		vec3 destination = vec3();
		Placement placeType = Placement::actor;

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
			//float deltaT = 1.0f;//static_cast<float>((clock() - timeCounter)/ 20);
			//timeCounter = clock();

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
				vec3 clickPosition((float)mouseX, (float)mouseY, 0.0f);

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
						boids.emplace_back(clickPosition, vec3(), spacePartition,
							vao, ib, actorTex, rTex, bTex, shader);
						break;
					case Placement::obstacle:
						obstacles.emplace_back(clickPosition);
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
				orthoWidth += zoomAmount * 5.0f;
			}
			//Imgui inputs
			{
				//static int counter = 0;
				ImGui::SliderFloat("Simulation speed", &simSpeed, 0.0f, 1.0f);
				ImGui::Text("Boid settings");
				ImGui::SliderFloat("Max Acceleration", &boidAcc, 0.01f, 0.2f);
				ImGui::SliderFloat("Max Speed", &boidSpeed, 0.01f, 1.0f);
				ImGui::SliderFloat("Home Bounds", &boidHome, 1.0f, 200.0f);
				ImGui::SliderFloat("View Arc", &boidView, 0.0f, 1.0f);
				ImGui::SliderFloat("Object Radius", &boidRadius, 0.1f, 20.0f);
				ImGui::SliderFloat("Avoidance Distance", &boidAvoid, 0.0f, 100.0f);
				ImGui::SliderFloat("Detection Distance", &boidDetect, 0.0f, 100.0f);
				ImGui::Checkbox("Damping", &boidDamping);
				ImGui::SameLine();
				ImGui::Checkbox("Draw avoidance", &drawAvoid);
				ImGui::SameLine();
				ImGui::Checkbox("Draw detection", &drawDetect);
				//ImGui::SliderFloat("Z", &translation.z, 100.0f, -100.0f);
				//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
				//ImGui::Checkbox("Another Window", &show_another_window);

				if (ImGui::Button("Restart"))
				{
					boids.clear();
					obstacles.clear();
					orthoHeight = 100.0f;
					orthoWidth = 100.0f;
					fillEntities(0, 0, boids, obstacles, spacePartition, vao, ib, 
						actorTex, rTex, bTex, shader);
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

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}

			//Start of rendering and simulation
			glm::mat4 view = glm::translate(glm::mat4(1.0f), translation);
			glm::mat4 projection = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -10.0f, 10.0f);
			glm::mat4 viewProjection = projection * view;

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
				boid.setMaxAcceleration(boidAcc);
				boid.setSpeed(boidSpeed);
				boid.setHomeDist(boidHome);
				boid.setViewArc(boidView);
				boid.setRadius(boidRadius);
				boid.setAvoidanceDist(boidAvoid);
				boid.setDetectionDist(boidDetect);
				boid.setDamping(boidDamping);
				boid.setHomeLocation(destination);
				//Run boid steering
				boid.steering(boids, obstacles);
				//Draw radii
				boid.drawAuras(renderer, viewProjection, drawAvoid, drawDetect);
			}
			
			for (Boid& boid : boids)
			{
				boid.locomotion(simSpeed);
				boid.draw(renderer, viewProjection);
			}

			for (vec3 obst : obstacles)
			{
				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(obst.x, obst.y, obst.z));
				glm::mat4 modelViewProjection = viewProjection * model;

				shader.bind();
				obstTex.bind(0);
				shader.setUniform1i("u_texture", 0);
				shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);

				renderer.draw(vao, ib, shader);
			}
			
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