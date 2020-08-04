#pragma once

#include "vec3.h"
#include "SpacePartition.h"

class Obstacle
{
private:
	SpacePartition& m_partition;
public:
	vec3 m_position;

	Obstacle(vec3 position, SpacePartition& partition)
		: m_position(position), m_partition(partition)
	{
		m_partition.addObstacle(position);
	}

	~Obstacle()
	{
		m_partition.removeObstacle(m_position);
	}
};