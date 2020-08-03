#pragma once

#include <vector>
#include <list>

class vec3;
class SpacePartition;
class Boid;

//Actor Steer Functions
namespace ASF
{
	//Adds a vector to another, capping the length of the second such that 
	//the result's length is 1 or less
	void accumulate(vec3& acc, vec3 add);
	//Collapses a vector to an infinite plane
	void flattenVectortoPlane(vec3& vector, vec3 plane);
	//Collects actor data from a list
	void dataCollectionFromList(vec3& sumPosition, vec3& sumVelocity, vec3& collision,
		int& count, float& closestDist, const Boid& self, const std::list<const Boid*>& boidList);
	//Collects actor data from a partition
	void actorDataCollection(vec3& sumPosition, vec3& sumVelocity, vec3& collision,
		const Boid& self, const SpacePartition& partition);
	//Collects obstacle data from a partition
	void obstacleDataCollection(vec3& collision, vec3 facingDirection, vec3 position, 
		float avoidanceDist, float radius, const std::vector<vec3>& obstacles);

	//Final steering activities

	//Avoid potential future collisions
	vec3 collisionAvoidance(vec3 collision, vec3 facingDirection);
	//Attempt to move within a given distance from the destination by the 
	//shortest route possible
	vec3 seekTowards(vec3 position, vec3 homeLocation, float homeDist, vec3 facingDirection);
	//Get the vector average velocity of the flock
	vec3 matchFlockVelocity(vec3 sumVelocity, float maxAcceleration, vec3 facingDirection);
	//Get the vector to the centre of the nearby flock
	vec3 matchFlockCentre(vec3 sumPosition, vec3 facingDirection);
};

