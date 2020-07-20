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

void obstacleDataCollection(vec3& sumCollision, const std::vector<vec3>& obstacles, const Boid& self)
{
	//Create temp storage of closest obstacle
	for (const vec3& obstacle : obstacles)
	{
		//Scale by facing direction
		//If outside near movement continue
		//If closest obstacle set as such and store relative position
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

vec3 collisionAvoidance(vec3 sumCollision, vec3 facingDirection)
{
	if (sumCollision != vec3())
	{
		float collisionMag = sumCollision.mag();
		vec3 avoidDirection = sumCollision - facingDirection.unit() * sumCollision.dot(facingDirection.unit());
		return avoidDirection.unit() * collisionMag;
	}
	return vec3();
}

vec3 seekTowards(vec3 position, float homeDist, vec3 facingDirection)
{
	vec3 homeVec = vec3() - position;
	if (homeVec.square() > homeDist * homeDist)
	{
		float mult = (homeVec.mag() - homeDist) / homeDist;
		vec3 avoidDirection = homeVec - facingDirection.unit() * homeVec.dot(facingDirection.unit());
		return avoidDirection.unit() * mult;
	}
	return vec3();
}

vec3 matchFlockVelocity(vec3 sumVelocity, float maxAcceleration, vec3 facingDirection)
{
	if (sumVelocity != vec3())
	{
		vec3 matchVel = sumVelocity / maxAcceleration;
		return sumVelocity - facingDirection.unit() * sumVelocity.dot(facingDirection.unit());
	}
}

vec3 matchFlockCentre(vec3 sumPosition, vec3 facingDirection)
{
	if (sumPosition != vec3())
	{
		vec3 matchPos = sumPosition;
		return sumPosition - facingDirection.unit() * sumPosition.dot(facingDirection.unit());
	}
	return vec3();
}

void Boid::update(std::vector<Boid>& boids, std::vector<vec3>& obstacles)
{
	vec3 oldAcceleration = m_acceleration;
	m_acceleration = vec3();
	vec3 facingDir = m_velocity.unit();
	//Find "flock" data
	vec3 sumPos = vec3();
	vec3 sumVel = vec3();
	vec3 sumCol = vec3();

	actorDataCollection(sumPos, sumVel, sumCol, boids, *this);
	obstacleDataCollection(sumCol, obstacles, *this);

	//Accumulating forces
	accumulate(m_acceleration, 
		collisionAvoidance(sumCol, facingDir));

	accumulate(m_acceleration, 
		seekTowards(m_position, m_homeDist, facingDir));
	
	accumulate(m_acceleration, 
		matchFlockVelocity(sumVel, m_maxAcceleration, facingDir));

	accumulate(m_acceleration,
		matchFlockCentre(sumPos, facingDir));

	//Damping
	if (m_damping)
		m_acceleration = (m_acceleration + oldAcceleration) / 2;

	//Ensure acceleration is perpendicular to velocity
	m_acceleration = m_acceleration - facingDir.unit() * m_acceleration.dot(facingDir.unit());
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
