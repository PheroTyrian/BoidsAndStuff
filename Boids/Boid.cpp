#include "Boid.h"
#include "Shader.h"
#include "VertexArray.h"
#include "glm/gtc/matrix_transform.hpp"
#include <cmath>
#include <random>

Boid::Boid(vec3 pos, vec3 vel, VertexArray& vao, IndexBuffer& ia, Texture& tex, Shader& shader)
	: m_position(pos), m_velocity(vel), m_acceleration(vec3()), m_maxAcceleration(1.0f), m_maxSpeed(10.0f), m_homeDist(100.0f),
	m_viewArc(0.75f), m_radius(2.0f), m_avoidanceDistance(5.0f), m_detectionDistance(10.0f), m_vao(vao), m_ib(ia), m_tex(tex), m_shader(shader)
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

void actorDataCollection(vec3& sumPosition, vec3& sumVelocity, vec3& collision, const std::vector<Boid>& boids, const Boid& self)
{
	//Create temp storage of closest collision
	float closestDist = self.getAvoidanceDist();
	if (collision != vec3())
		closestDist = collision.square();

	for (const Boid& boid : boids)
	{
		//rather than creating the flock store the average of nearby velocities and positions simultaneously as it saves on temp data
		vec3 diff = boid.getPosition() - self.getPosition();

		//Don't count self
		if (diff == vec3())
			continue;

		//Blind behind
		float sigma = diff.dot(self.getVelocity()) / (diff.mag() * self.getVelocity().mag());
		if (std::acos(sigma) > 3.1416 * self.getViewArc())
			continue;

		//Neighbour data
		if (diff.mag() < self.getDetectionDist())
		{
			sumPosition += boid.getPosition();
			sumVelocity += boid.getVelocity() - self.getVelocity();
		}

		//Collision checking
		//Find position of closest intercept in near future (midpoint between the two closest points bounded between 0 and nearFuture)
		float nearFuture = self.getAvoidanceDist() / self.getMaxSpeed();
		float steps = self.getAvoidanceDist() / (self.getRadius() * 0.25f); //Should be based on the radius of the object

		//Determine if it's close enough to care
		if (nearFuture * self.getMaxSpeed() > self.getAvoidanceDist() + diff.mag())
			continue;

		//Check iteratively for collisions
		for (float t = 0.0f; t <= nearFuture; t += (nearFuture / steps))
		{
			vec3 selfPosition = self.getPosition() + (self.getVelocity() * t);
			vec3 otherPosition = boid.getPosition() + (boid.getVelocity() * t);
			float potentialClosest = (otherPosition - self.getPosition()).mag();
			
			//Move on if this will not provide a closer collision than has already been detected
			if (potentialClosest > closestDist)
				continue;

			if ((otherPosition - selfPosition).mag() <= self.getRadius() + boid.getRadius())
			{
				closestDist = potentialClosest;
				//This treats the potential moving collision target as a static object at the intercept
				collision = otherPosition - self.getPosition();
			}
		}
	}
}

//void obstacleDataCollection(vec3& sumCollision, const std::vector<vec3>& obstacles, const Boid& self)
//{
//	for (const vec3& obstacle : obstacles)
//	{
//		vec3 diff = obstacle - self.getPosition();
//
//		if (diff == vec3())
//			continue;
//
//		if (diff.square() < self.getAvoidanceDist() * self.getAvoidanceDist())
//		{
//			float avoidScale = self.getAvoidanceDist() * self.getAvoidanceDist() - diff.square();
//			//A vector is produced in the opposite direction to the object and of a size proportional to how close it is
//			sumCollision -= diff.unit() * avoidScale;
//		}
//	}
//}

void obstacleDataCollection(vec3& collision, vec3 facingDirection, vec3 position, float avoidanceDist, float radius, const std::vector<vec3>& obstacles)
{
	//Create temp storage of closest obstacle
	float closestDist = avoidanceDist;
	if (collision != vec3())
		closestDist = collision.mag();

	for (const vec3& obstacle : obstacles)
	{
		vec3 diff = obstacle - position;
		
		//Scale by facing direction
		float distForward = facingDirection.dot(diff);

		//Cull results outside box ends
		if (distForward <= 0 || distForward > avoidanceDist)
			continue;
		
		//Cull results too far from the sides
		if ((diff - (facingDirection * distForward)).mag() > radius)
			continue;

		//If closest obstacle set as such and store relative position
		if (closestDist > diff.mag())
		{
			closestDist = diff.mag();
			collision = diff;
		}
	}
}

vec3 collisionAvoidance(vec3 collision, vec3 facingDirection)
{
	if (collision != vec3())
	{
		vec3 avoidDirection = vec3() - (collision - facingDirection * collision.dot(facingDirection));
		return avoidDirection.unit();
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
	return vec3();
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

void Boid::steering(std::vector<Boid>& boids, std::vector<vec3>& obstacles)
{
	vec3 oldAcceleration = m_acceleration;
	m_acceleration = vec3();
	vec3 facingDir = m_velocity.unit();
	//Find "flock" data
	vec3 sumPos = vec3();
	vec3 sumVel = vec3();
	vec3 sumCol = vec3();

	actorDataCollection(sumPos, sumVel, sumCol, boids, *this);
	obstacleDataCollection(sumCol, facingDir, m_position, m_avoidanceDistance, m_radius, obstacles);

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

void Boid::locomotion(float deltaT)
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
