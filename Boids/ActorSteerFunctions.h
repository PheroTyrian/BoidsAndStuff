#pragma once

#include "Shape.h"
#include <vector>
#include <list>

class vec3;
class SpacePartition;
class Boid;
class Obstacle;

//Actor Steer Functions
namespace ASF
{
	//Utility

	//Adds a vector to another, capping the length of the second such that 
	//the result's length is 1 or less
	void accumulate(vec3& acc, vec3 add);
	//Collapses a vector to a plane
	void flattenVectortoPlane(vec3& vector, vec3 plane);

	//Data collection

	//Collects actor and obstacle data from the area surrounding an actor
	void actorDataCollection(vec3& sumPosition, vec3& sumVelocity, vec3& collision,
		const Boid& self, const SpacePartition& partition);
	//Collects regions of undesirable velocity for use by the clearPathSampling
	std::list<Shape>& velocityObstacleCollection(const Boid& self,
		const SpacePartition& partition);

	//Final steering activities

	//Avoid potential future collisions
	vec3 simpleCollisionAvoidance(vec3 closestCollision, vec3 facingDirection);
	vec3 clearPathSampling(vec3 targetVelocity, float maxVel, 
		std::list<Shape>& velocityObstacles);
	//Attempt to move within a given distance from the destination by the 
	//shortest route possible
	vec3 seekTowards(vec3 position, vec3 homeLocation, float homeDist, vec3 facingDirection);
	//Get the vector average velocity of the flock
	vec3 matchFlockVelocity(vec3 sumVelocity, float maxAcceleration, vec3 facingDirection);
	//Get the vector to the centre of the nearby flock
	vec3 matchFlockCentre(vec3 sumPosition, vec3 facingDirection);
};

