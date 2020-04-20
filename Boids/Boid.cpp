#include "Boid.h"
#include "Shader.h"
#include "VertexArray.h"
#include "glm/gtc/matrix_transform.hpp"
#include <cmath>
#include <random>

Boid::Boid(vec3 pos, vec3 vel, float acc, float drag, float avoid, float detect, VertexArray& vao, IndexBuffer& ia, Texture& tex, Shader& shader)
	: m_position(pos), m_velocity(vel), m_acceleration(vec3()), m_maxAcceleration(acc), m_dragEffect(drag), 
	m_avoidanceDistance(avoid), m_detectionDistance(detect), m_vao(vao), m_ib(ia), m_tex(tex), m_shader(shader)
{
}

Boid::~Boid()
{
}

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
		vec3 diff = m_position - boid.m_position;
		if (diff.square() < m_detectionDistance)
		{
			if (diff == vec3())
				continue;

			sumPos += boid.m_position;
			sumVel += boid.m_velocity;
		}
	}
	sumPos = sumPos.unit();

	//Collision avoidance
	vec3 collision = collisionAvoidance(obstacles, m_position, m_avoidanceDistance);

	//Match velocity with flock
	sumVel = sumVel.unit();
	vec3 matchVel = (sumVel - m_velocity) / m_maxAcceleration;

	//Move toward flock centre
	sumPos = sumPos.unit();
	vec3 matchPos = (sumPos - m_position) / m_detectionDistance;

	//Random acceleration
	vec3 randmotion = vec3(rand() % 21 - 10, rand() % 21 - 10, rand() % 21 - 10);
	randmotion = randmotion.unit();

	//Accumulate
	m_acceleration = collision;
	accumulate(m_acceleration, matchVel);
	accumulate(m_acceleration, matchPos);
	accumulate(m_acceleration, randmotion);
}

void Boid::simulate(float deltaT)
{
	m_velocity += m_acceleration * m_maxAcceleration * deltaT;
	//Temp code capping velocity in place of drag
	float magnitude = std::min(m_velocity.mag(), 5.0f);
	m_velocity = m_velocity.unit() * magnitude;

	m_position += m_velocity * deltaT;
	//Temp code wrapping positions
	if (m_position.x > 100.0f)
		m_position.x -= 200.0f;
	if (m_position.y > 100.0f)
		m_position.y -= 200.0f;
	if (m_position.z > 100.0f)
		m_position.z -= 200.0f;

	if (m_position.x < -100.0f)
		m_position.x += 200.0f;
	if (m_position.y < -100.0f)
		m_position.y += 200.0f;
	if (m_position.z < -100.0f)
		m_position.z += 200.0f;
}

void Boid::draw(Renderer & renderer, glm::mat4 viewProjection)
{
	m_shader.bind();
	m_tex.bind(0);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_position.x, m_position.y, m_position.z));
	glm::mat4 modelViewProjection = viewProjection * model;

	m_shader.setUniform1i("u_texture", 0);
	m_shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);

	renderer.draw(m_vao, m_ib, m_shader);
}
