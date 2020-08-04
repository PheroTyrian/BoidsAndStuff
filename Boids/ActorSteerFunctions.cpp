#include "ActorSteerFunctions.h"
#include "vec3.h"
#include "Boid.h"
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
			collectionFromActors(sumPosition, sumVelocity, collision,
				sumCount, closestDist, self, partition.getCell(x, y).actors);
			collectionFromObstacles(collision, facing, self.getPosition(),
				self.getAvoidanceDist(), self.getRadius(), partition.getCell(x, y).obstacles);
		}
	}
	if (range.incOOB)
	{
		collectionFromActors(sumPosition, sumVelocity, collision,
			sumCount, closestDist, self, partition.getOOB().actors);
		collectionFromObstacles(collision, facing, self.getPosition(),
			self.getAvoidanceDist(), self.getRadius(), partition.getOOB().obstacles);
	}
	sumPosition / sumCount;
	sumVelocity / sumCount;
}

void ASF::collectionFromActors(vec3& sumPosition, vec3& sumVelocity, vec3& collision,
	int& count, float& closestDist, const Boid& self, const std::list<const Boid*>& boidList)
{
	for (const Boid* boid : boidList)
	{
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

void ASF::collectionFromObstacles(vec3& collision, vec3 facingDirection, vec3 position,
	float avoidanceDist, float radius, const std::list<vec3>& obstList)
{
	//Create temp storage of closest obstacle
	float closestDist = avoidanceDist;
	if (collision != vec3())
		closestDist = collision.mag();

	for (const vec3& obstacle : obstList)
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

vec3 ASF::collisionAvoidance(vec3 collision, vec3 facingDirection)
{
	if (collision != vec3())
	{
		vec3 avoidDirection = vec3() - (collision - facingDirection * collision.dot(facingDirection));
		return avoidDirection.unit();
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