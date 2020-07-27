#pragma once

#include <vector>
#include <list>
#include "vec3.h"

class Boid;

class SpacePartition
{
private:
	int m_storedObjects;
	int m_sizeX, m_sizeY;
	float m_partitionWidth;
	vec3 m_bottomLeft;
	vec3 m_topRight;
	std::vector<std::list<const Boid*>> m_Partitions;
	std::list<const Boid*> m_oob;

public:
	bool isOutOfBounds(vec3 position);

	std::list<const Boid*>& findCell(vec3 position);

	void add(const Boid* boid);

	void haveMoved(const Boid* boid, vec3 oldPosition);

	SpacePartition(int sizeX, int sizeY, float partitionWidth);

	~SpacePartition();
};