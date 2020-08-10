#include "ActorSteerFunctions.h"
#include "vec3.h"
#include "Boid.h"
#include "Obstacle.h"
#include "SpacePartition.h"
#include <vector>
#include <list>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace ASF;

//Adds a vector to another, capping the length of the second such that the result's length is 1 or less
void ASF::accumulate(vec3& acc, vec3 add)
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

void ASF::flattenVectortoPlane(vec3& vector, vec3 plane)
{
	plane = plane.unit();
	vector -= plane * plane.dot(vector);
}

//Helper for actorDataCollection. Collects actor data from a list
void collectFromActors(vec3& sumPosition, vec3& sumVelocity, vec3& collision,
	int& count, float& closestDist, const Boid& self, const std::list<const Boid*>& boidList)
{
	for (const Boid* boid : boidList)
	{
		if (!boid)
			continue;
		//rather than creating the flock store the average of nearby velocities and positions simultaneously as it saves on temp data
		vec3 diff = boid->getPosition() - self.getPosition();

		//Don't count self
		if (diff == vec3())
			continue;

		//Blind behind
		float sigma = diff.dot(self.getVelocity()) / (diff.mag() * self.getVelocity().mag());
		if (acos(sigma) > M_PI * self.getViewArc())
			continue;

		//Neighbour data
		if (diff.mag() < self.getDetectionDist())
		{
			sumPosition += boid->getPosition();
			sumVelocity += boid->getVelocity() - self.getVelocity();
			count++;
		}

		//Collision checking
		//Find position of closest intercept in near future (midpoint between the two closest points bounded between 0 and nearFuture)
		float nearFuture = self.getAvoidanceDist() / self.getMaxSpeed();
		float steps = self.getAvoidanceDist() / self.getRadius(); //Should be based on the radius of the object

		//Determine if it's close enough to care
		if (self.getAvoidanceDist() < diff.mag())
			continue;

		//Check iteratively for collisions
		for (float t = 0.0f; t <= nearFuture; t += (nearFuture / steps))
		{
			vec3 selfPosition = self.getPosition() + (self.getVelocity() * t);
			vec3 otherPosition = boid->getPosition() + (boid->getVelocity() * t);
			float potentialClosest = (otherPosition - self.getPosition()).mag();

			//Move on if this will not provide a closer collision than has already been detected
			if (potentialClosest > closestDist)
				continue;

			if ((otherPosition - selfPosition).mag() <= self.getRadius() + boid->getRadius())
			{
				closestDist = potentialClosest;
				//This treats the potential moving collision target as a static object at the intercept
				collision = otherPosition - self.getPosition();
			}
		}
	}
}

//Helper for actorDataCollection. Collects obstacle data from a list
void collectFromObstacles(vec3& collision, vec3 facingDirection, vec3 position,
	float avoidanceDist, float radius, const std::list<const Obstacle*>& obstList)
{
	//Create temp storage of closest obstacle
	float closestDist = avoidanceDist;
	if (collision != vec3())
		closestDist = collision.mag();

	for (const Obstacle* obstacle : obstList)
	{
		if (!obstacle)
			continue;

		vec3 diff = obstacle->m_position - position;

		//Scale by facing direction
		float distForward = facingDirection.dot(diff);

		//Cull results outside box ends
		if (distForward <= 0 || distForward > avoidanceDist)
			continue;

		//Cull results too far from the sides
		if ((diff - (facingDirection * distForward)).mag() > radius + obstacle->m_radius)
			continue;

		//If closest obstacle set as such and store relative position
		if (closestDist > diff.mag())
		{
			closestDist = diff.mag();
			collision = diff;
		}
	}
}

void ASF::actorDataCollection(vec3& sumPosition, vec3& sumVelocity, vec3& collision,
	const Boid& self, const SpacePartition& partition)
{
	//Create temp storage of closest collision
	float closestDist = self.getAvoidanceDist();
	if (collision != vec3())
		closestDist = collision.square();
	int sumCount = 0;
	vec3 facing = self.getVelocity().unit();
	//Find the search region
	CellRange range = partition.findCellRange(self.getPosition(), std::max(self.getDetectionDist(), self.getAvoidanceDist()));

	//Check through each cell in range
	for (int y = range.blY; y < range.trY; y++)
	{
		for (int x = range.blX; x < range.trX; x++)
		{
			collectFromActors(sumPosition, sumVelocity, collision,
				sumCount, closestDist, self, partition.getCell(x, y).actors);
			collectFromObstacles(collision, facing, self.getPosition(),
				self.getAvoidanceDist(), self.getRadius(), partition.getCell(x, y).obstacles);
		}
	}
	if (range.incOOB)
	{
		collectFromActors(sumPosition, sumVelocity, collision,
			sumCount, closestDist, self, partition.getOOB().actors);
		collectFromObstacles(collision, facing, self.getPosition(),
			self.getAvoidanceDist(), self.getRadius(), partition.getOOB().obstacles);
	}
	sumPosition / sumCount;
	sumVelocity / sumCount;
}

void getActorVOs(vec3 position, vec3 velocity, float avoidDist, float radius,
	std::list<Shape>& velObsts, const std::list<const Boid*>& boidList)
{
	for (const Boid* boid : boidList)
	{
		if (!boid)
			continue;

		vec3 diff = boid->getPosition() - position;

		if (diff.mag() > avoidDist)
			continue;

		vec3 velPos = (velocity + boid->getVelocity()) / 2;

		Shape tempVO = Shape(velPos);
		//Create a cone of vectors that intersect the boid
		tempVO.addConeSection(diff, radius, boid->getRadius(), avoidDist * 10.0f);
		//Add the rough shape of self to this viathe Minkowsky sum
		tempVO.addSquare(velocity.unit(), radius);

		velObsts.push_back(tempVO);
	}
}

void getObstacleVOs(vec3 position, vec3 velocity, float avoidDist, float radius,
	std::list<Shape>& velObsts, const std::list<const Obstacle*>& obstList)
{
	for (const Obstacle* obst : obstList)
	{
		if (!obst)
			continue;

		vec3 diff = obst->m_position - position;

		if (diff.mag() > avoidDist)
			continue;

		vec3 velPos = velocity / 2;

		Shape tempVO = Shape(velPos);
		//Create a cone of vectors that intersect the obstacle
		tempVO.addConeSection(diff, radius, obst->m_radius, avoidDist * 100.0f);
		//Add the rough shape of self to this viathe Minkowsky sum
		tempVO.addSquare(velocity.unit(), radius);

		velObsts.push_back(tempVO);
	}
}

void ASF::velocityObstacleCollection(const Boid& self, std::list<Shape>& velocityObstacles,
	const SpacePartition& partition)
{
	//Create a list of shapes and gather common data
	vec3 pos = self.getPosition();
	vec3 vel = self.getVelocity();
	float avoid = self.getAvoidanceDist();
	float radius = self.getRadius();

	//For all nearby boids and obstacles create a VO and translate by (v1 + v2) / 2
	CellRange range = partition.findCellRange(self.getPosition(), self.getAvoidanceDist());
	for (int y = range.blY; y < range.trY; y++)
	{
		for (int x = range.blX; x < range.trX; x++)
		{
			getActorVOs(pos, vel, avoid, radius, velocityObstacles, partition.getCell(x, y).actors);
			getObstacleVOs(pos, vel, avoid, radius, velocityObstacles, partition.getCell(x, y).obstacles);
		}
	}
	if (range.incOOB)
	{
		getActorVOs(pos, vel, avoid, radius, velocityObstacles, partition.getOOB().actors);
		getObstacleVOs(pos, vel, avoid, radius, velocityObstacles, partition.getOOB().obstacles);
	}
}

vec3 ASF::simpleCollisionAvoidance(vec3 collision, vec3 facingDirection)
{
	if (collision != vec3())
	{
		vec3 avoidDirection = vec3() - (collision - facingDirection * collision.dot(facingDirection));
		return avoidDirection.unit();
	}
	return vec3();
}

vec3 ASF::clearPathSampling(vec3 targetAcceleration, vec3 currentVel, float maxVel, 
	std::list<Shape>& velocityObstacles)
{
	vec3 samples[6];
	//Construct set of potential velocities to sample
	{
		vec3 targetVel = currentVel + targetAcceleration;
		float targetAngle = atan2(targetVel.y, targetVel.x);
		float facingAngle = atan2(currentVel.y, currentVel.x);
		float angle1 = facingAngle + M_PI / 4;
		float angle2 = facingAngle - M_PI / 4;
		float angle3 = facingAngle + M_PI / 2;
		float angle4 = facingAngle - M_PI / 2;
		samples[0] = targetVel.unit() * maxVel;
		samples[1] = targetVel.unit() * maxVel;
		samples[2] = vec3(cos(angle1), sin(angle1), 0.0f) * maxVel;
		samples[3] = vec3(cos(angle2), sin(angle2), 0.0f) * maxVel;
		samples[4] = vec3(cos(angle3), sin(angle3), 0.0f) * maxVel;
		samples[5] = vec3(cos(angle4), sin(angle4), 0.0f) * maxVel;
	}
	//Gradually reduce vector length until a solution is found
	for (float scale = 1.0f; scale > 0.0f; scale -= 0.1f)
	{
		//Test each sample vec for collision against VOs
		bool sampleSuccess[6] = { true, true, true, true, true, true };
		for (Shape& vo : velocityObstacles)
		{
			for (int i = 0; i < 6; ++i)
			{
				if (sampleSuccess[i])
					sampleSuccess[i] = !vo.isPointInside(samples[i] * scale);
			}
		}
		//The first that doesn't collide with any is taken as the result
		//Samples are ordered with faster movement options and directions prefered by other 
		//steering functions given precedence
		for (int i = 0; i < 6; ++i)
		{
			if (sampleSuccess[i])
			{
				vec3 velocitySuggestion = samples[i] * scale;
				return velocitySuggestion - currentVel;
			}
		}
	}
	return vec3();
}

vec3 ASF::seekTowards(vec3 position, vec3 homeLocation, float homeDist, vec3 facingDirection)
{
	vec3 homeVec = homeLocation - position;
	if (homeVec.mag() > homeDist)
	{
		float mult = (homeVec.mag() - homeDist) / homeDist;
		vec3 avoidDirection = homeVec - facingDirection.unit() * homeVec.dot(facingDirection.unit());
		return avoidDirection.unit() * mult;
	}
	return vec3();
}

vec3 ASF::matchFlockVelocity(vec3 sumVelocity, float maxAcceleration, vec3 facingDirection)
{
	if (sumVelocity != vec3())
	{
		vec3 matchVel = sumVelocity / maxAcceleration;
		matchVel = matchVel - facingDirection.unit() * matchVel.dot(facingDirection.unit());
		if (matchVel.mag() > 1.0f)
			matchVel = matchVel.unit();
		return matchVel;
	}
	return vec3();
}

vec3 ASF::matchFlockCentre(vec3 sumPosition, vec3 facingDirection)
{
	if (sumPosition != vec3())
	{
		vec3 matchPos = sumPosition - facingDirection.unit() * sumPosition.dot(facingDirection.unit());
		if (matchPos.mag() > 1.0f)
			matchPos = matchPos.unit();
		return matchPos;
	}
	return vec3();
}