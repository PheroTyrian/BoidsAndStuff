#pragma once

#include <vector>
#include <list>
#include "vec3.h"

class Boid;

class SpacePartition
{
	int m_storedObjects;
	int m_sizeX, m_sizeY;
	float m_partitionWidth;
	vec3 m_bottomLeft;
	std::vector<std::list<Boid*>> m_Partitions;
	std::list<Boid*> m_oob

	SpacePartition(int sizeX, int sizeY, float partitionWidth) : m_storedObjects(0), m_sizeX(sizeX), m_sizeY(sizeY), m_partitionWidth(partitionWidth), m_topLeft(vec3())
	{
		m_bottomLeft = vec3(-sizeX * partitionWidth / 2, -sizeY * partitionWidth / 2, 0);
		m_Partitions.resize(sizeX * sizeY);
	}
};