#pragma once

#include "vec3.h"
#include "SpacePartition.h"

class Obstacle
{
private:
	SpacePartition& m_partition;
public:
	vec3 m_position;
	float m_radius;

	Obstacle(vec3 position, float radius, SpacePartition& partition)
		: m_position(position), m_radius(radius), m_partition(partition)
	{
		m_partition.addObstacle(this);
	}

	~Obstacle()
	{
		m_partition.removeObstacle(this);
	}
};