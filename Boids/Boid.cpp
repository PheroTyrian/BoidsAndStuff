#include "Boid.h"
#include "Shader.h"
#include "VertexArray.h"
#include "glm/gtc/matrix_transform.hpp"
#include <cmath>
#include <random>

Boid::Boid(vec3 pos, vec3 vel, float maxAcc, float speed, float home, float viewArc, float avoid, float detect, VertexArray& vao, IndexBuffer& ia, Texture& tex, Shader& shader)
	: m_position(pos), m_velocity(vel), m_acceleration(vec3()), m_maxAcceleration(maxAcc), m_maxSpeed(speed), m_homeDist(home),
	m_viewArc(viewArc), m_avoidanceDistance(avoid), m_detectionDistance(detect), m_vao(vao), m_ib(ia), m_tex(tex), m_shader(shader)
{
}

Boid::~Boid()
{
}

//Start of removal area

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

void flattenVectortoPlane(vec3& vector, vec3 plane)
{
	plane = plane.unit();
	vector -= plane * plane.dot(vector);
}

void actorDataCollection(vec3& sumPosition, vec3& sumVelocity, vec3& sumCollision, const std::vector<Boid>& boids, const Boid& self)
{
	for (const Boid& boid : boids)
	{
		//rather than creating the flock store the average of nearby velocities and positions simultaneously as it saves on temp data
		vec3 diff = boid.getPosition() - self.getPosition();
		//Don't count self
		if (diff == vec3())
			continue;

		if (diff.square() < self.getDetectionDist() * self.getDetectionDist())
		{
			//Blind behind
			float sigma = diff.dot(self.getVelocity()) / (diff.mag() * self.getVelocity().mag());
			if (std::acos(sigma) > 3.1416 * self.getViewArc())
				continue;
			sumPosition += boid.getPosition();
			sumVelocity += boid.getVelocity() - self.getVelocity();
		}

		if (diff.square() < self.getAvoidanceDist() * self.getAvoidanceDist())
		{
			float avoidScale = self.getAvoidanceDist() * self.getAvoidanceDist() - diff.square();
			//A vector is produced in the opposite direction to the object and of a size proportional to how close it is
			sumCollision -= diff.unit() * avoidScale;
		}
	}
}

void obstacleDataCollection(vec3& sumCollision, const std::vector<vec3>& obstacles, const Boid& self)
{
	for (const vec3& obstacle : obstacles)
	{
		vec3 diff = obstacle - self.getPosition();

		if (diff == vec3())
			continue;

		if (diff.square() < self.getAvoidanceDist() * self.getAvoidanceDist())
		{
			float avoidScale = self.getAvoidanceDist() * self.getAvoidanceDist() - diff.square();
			//A vector is produced in the opposite direction to the object and of a size proportional to how close it is
			sumCollision -= diff.unit() * avoidScale;
		}
	}
}

void Boid::update(std::vector<Boid>& boids, std::vector<vec3>& obstacles)
{
	vec3 oldAcceleration = m_acceleration;
	m_acceleration = vec3();
	vec3 facingDirection = m_velocity.unit();
	//Find "flock" data
	vec3 sumPos = vec3();
	vec3 sumVel = vec3();
	vec3 sumCol = vec3();

	actorDataCollection(sumPos, sumVel, sumCol, boids, *this);
	obstacleDataCollection(sumCol, obstacles, *this);

	//Collision avoidance
	if (sumCol != vec3())
	{
		float collisionMag = sumCol.mag();
		vec3 avoidDirection = sumCol - facingDirection.unit() * sumCol.dot(facingDirection.unit());
		accumulate(m_acceleration, avoidDirection.unit() * collisionMag);
	}

	//Home towards 0, 0
	vec3 homeVec = vec3() - m_position;
	if (homeVec.square() > m_homeDist * m_homeDist)
	{
		float mult = (homeVec.mag() - m_homeDist) / m_homeDist;
		vec3 avoidDirection = homeVec - facingDirection.unit() * homeVec.dot(facingDirection.unit());
		accumulate(m_acceleration, avoidDirection.unit() * mult);
	}

	//Match velocity with flock
	if (sumVel != vec3())
	{
		vec3 matchVel = sumVel / m_maxAcceleration;
		vec3 avoidDirection = sumVel - facingDirection.unit() * sumVel.dot(facingDirection.unit());
		accumulate(m_acceleration, avoidDirection);
	}

	//Move toward flock centre
	if (sumPos != vec3())
	{
		vec3 matchPos = sumPos;
		vec3 avoidDirection = sumPos - facingDirection.unit() * sumPos.dot(facingDirection.unit());
		accumulate(m_acceleration, matchPos);
	}

	//Random acceleration
	//vec3 randmotion = vec3(rand() % 21 - 10, rand() % 21 - 10, 0.0f);
	//randmotion = randmotion.unit();
	//accumulate(m_acceleration, randmotion * 0.5f);

	//Damping
	if (m_damping)
		m_acceleration = (m_acceleration + oldAcceleration) / 2;

	//Ensure acceleration is perpendicular to velocity
	m_acceleration = m_acceleration - facingDirection.unit() * m_acceleration.dot(facingDirection.unit());
}

void Boid::simulate(float deltaT)
{
	m_velocity += m_acceleration * m_maxAcceleration * deltaT;
	
	m_velocity = m_velocity.unit() * m_maxSpeed;

	m_position += m_velocity * deltaT;
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

	glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angle, pivot);
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(m_position.x, m_position.y, m_position.z));

	glm::mat4 model = translate * rotate;
	glm::mat4 modelViewProjection = viewProjection * model;

	m_shader.setUniform1i("u_texture", 0);
	m_shader.setUniformMat4f("u_modelViewProjection", modelViewProjection);

	renderer.draw(m_vao, m_ib, m_shader);
}
