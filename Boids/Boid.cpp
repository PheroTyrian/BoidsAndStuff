#include "Boid.h"
#include "Shader.h"
#include "VertexArray.h"
#include "SpacePartition.h"
#include "glm/gtc/matrix_transform.hpp"
#include "ActorSteerFunctions.h"
#include <cmath>
#include <random>

Boid::Boid(vec3 pos, vec3 vel, SpacePartition& partition, VertexArray& vao, IndexBuffer& ia, Texture& tex, Texture& outlineTexR, Texture& outlineTexB, Shader& shader)
	: m_position(pos), m_velocity(vel), m_acceleration(vec3()), m_homeLocation(vec3()), m_maxAcceleration(1.0f), m_maxSpeed(10.0f), m_homeDist(100.0f),
	m_viewArc(0.75f), m_radius(2.0f), m_avoidanceDistance(5.0f), m_detectionDistance(10.0f), m_partition(partition), m_vao(vao),
	m_ib(ia), m_tex(tex), m_outlineR(outlineTexR), m_outlineB(outlineTexB), m_shader(shader)
{
	m_partition.add(this);
}

Boid::~Boid()
{
	m_partition.remove(this);
}

void Boid::steering(std::vector<Boid>& boids, std::vector<vec3>& obstacles)
{
	vec3 oldAcceleration = m_acceleration;
	m_acceleration = vec3();
	vec3 facingDir = m_velocity.unit();
	//Find "flock" data
	vec3 sumPos = vec3();
	vec3 sumVel = vec3();
	vec3 sumCol = vec3();

	ASF::actorDataCollection(sumPos, sumVel, sumCol, *this, m_partition);
	ASF::obstacleDataCollection(sumCol, facingDir, m_position, m_avoidanceDistance, m_radius, obstacles);

	//Accumulating forces
	ASF::accumulate(m_acceleration,
		ASF::collisionAvoidance(sumCol, facingDir));
	
	ASF::accumulate(m_acceleration,
		ASF::matchFlockVelocity(sumVel, m_maxAcceleration, facingDir));

	ASF::accumulate(m_acceleration,
		ASF::matchFlockCentre(sumPos, facingDir));

	ASF::accumulate(m_acceleration,
		ASF::seekTowards(m_position, m_homeLocation, m_homeDist, facingDir));

	//Damping
	if (m_damping)
		m_acceleration = (m_acceleration + oldAcceleration) / 2;

	//Ensure acceleration is perpendicular to velocity
	m_acceleration = m_acceleration - facingDir.unit() * m_acceleration.dot(facingDir.unit());
}

void Boid::locomotion(float deltaT)
{
	m_velocity += m_acceleration * m_maxAcceleration * deltaT;
	
	m_velocity = m_velocity.unit() * m_maxSpeed;

	vec3 oldPosition = m_position;

	m_position += m_velocity * deltaT;

	m_partition.haveMoved(this, oldPosition);
}

void Boid::draw(Renderer & renderer, glm::mat4 viewProjection)
{
	m_shader.bind();
	m_tex.bind(0);

	//Get the rotation pivot
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 vel = glm::normalize(glm::vec3(m_velocity.x, m_velocity.y, m_velocity.z));
	glm::vec3 pivot = glm::normalize(glm::cross(worldUp, vel));

	//Get rotation amount
	float angle = glm::acos(glm::dot(vel, worldUp));

	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_radius/2));
	glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angle, pivot);
	glm::mat4 translate = glm::translate(
		glm::mat4(1.0f), glm::vec3(m_position.x, m_position.y, m_position.z));

	glm::mat4 model = translate * rotate * scale;
	glm::mat4 modelViewProjection = viewProjection * model;

	m_shader.setUniform1i("u_texture", 0);
	m_shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);

	renderer.draw(m_vao, m_ib, m_shader);
}

void Boid::drawAuras(
	Renderer& renderer, glm::mat4 viewProjection, bool drawAvoid, bool drawDetect)
{
	m_shader.bind();
	if (drawAvoid)
	{
		m_outlineR.bind(0);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_avoidanceDistance/2));
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_position.x, m_position.y, m_position.z));
		glm::mat4 modelViewProjection = viewProjection * model * scale;
		m_shader.setUniform1i("u_texture", 0);
		m_shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);
		renderer.draw(m_vao, m_ib, m_shader);
	}
	if (drawDetect)
	{
		m_outlineB.bind(0);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_detectionDistance/2));
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_position.x, m_position.y, m_position.z));
		glm::mat4 modelViewProjection = viewProjection * model * scale;
		m_shader.setUniform1i("u_texture", 0);
		m_shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);
		renderer.draw(m_vao, m_ib, m_shader);
	}
}
