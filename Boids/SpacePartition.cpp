#include "SpacePartition.h"
#include "Boid.h"

#include <cmath>


bool SpacePartition::isOutOfBounds(vec3 position)
{
	if (position.x >= m_bottomLeft.x && 
		position.y >= m_bottomLeft.y &&
		position.x < m_topRight.x &&
		position.y < m_topRight.y)
		return false;
	else
		return true;
}

std::list<const Boid*>& SpacePartition::findCell(vec3 position)
{
	if (isOutOfBounds(position))
		return m_oob;
	else
	{
		vec3 unrounded = (position - m_bottomLeft) / m_partitionWidth;
		int cellX = std::floor(unrounded.x);
		int cellY = std::floor(unrounded.y);
		return m_Partitions[cellX + (m_sizeX * cellY)];
	}
}

void SpacePartition::add(const Boid* boid)
{
	if (!boid)
		return;

	vec3 position = boid->getPosition();

	findCell(position).push_back(boid);
}

void SpacePartition::haveMoved(const Boid* boid, vec3 oldPosition)
{
	if (!boid)
		return;

	vec3 position = boid->getPosition();

	std::list<const Boid*>& newCell = findCell(position);
	std::list<const Boid*>& oldCell = findCell(oldPosition);

	if (findCell(position) == findCell(oldPosition))
		return;
	else
	{
		oldCell.remove(boid);
		newCell.push_back(boid);
	}
}

SpacePartition::SpacePartition(int sizeX, int sizeY, float partitionWidth) 
	: m_storedObjects(0), m_sizeX(sizeX), m_sizeY(sizeY), m_partitionWidth(partitionWidth), m_bottomLeft(vec3())
{
	m_bottomLeft = vec3(-sizeX * partitionWidth / 2, -sizeY * partitionWidth / 2, 0);
	m_topRight = vec3(m_bottomLeft.x + (partitionWidth * sizeX), m_bottomLeft.x + (partitionWidth * sizeX), 0);
	m_Partitions.resize(sizeX * sizeY);
}

SpacePartition::~SpacePartition()
{
}
