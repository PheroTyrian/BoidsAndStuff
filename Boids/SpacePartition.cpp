#include "SpacePartition.h"
#include "Boid.h"

#include <cmath>
#include <algorithm>


bool SpacePartition::isOutOfBounds(int x, int y) const
{
	if (x >= 0 &&
		y >= 0 &&
		x < m_sizeX &&
		y < m_sizeY)
		return false;
	else
		return true;
}

bool SpacePartition::isOutOfBounds(vec3 position) const
{
	if (position.x >= m_bottomLeft.x && 
		position.y >= m_bottomLeft.y &&
		position.x < m_topRight.x &&
		position.y < m_topRight.y)
		return false;
	else
		return true;
}

const SpacePartition::Cell& SpacePartition::getCell(int x, int y) const
{
	if (isOutOfBounds(x, y))
		return m_oob;
	else
		return m_partitions[x + (y * m_sizeX)];
}

SpacePartition::Cell& SpacePartition::getCell(vec3 position)
{
	if (isOutOfBounds(position))
		return m_oob;
	else
	{
		vec3 unrounded = (position - m_bottomLeft) / m_partitionWidth;
		int cellX = std::floor(unrounded.x);
		int cellY = std::floor(unrounded.y);
		return m_partitions[cellX + (cellY * m_sizeX)];
	}
}

CellRange SpacePartition::findCellRange(vec3 position, float radius) const
{
	bool oob = false;
	//Fit to ints
	int blX = std::floor((position.x - radius - m_bottomLeft.x) / m_partitionWidth);
	int blY = std::floor((position.y - radius - m_bottomLeft.y) / m_partitionWidth);
	int trX = std::floor((position.x + radius - m_bottomLeft.x) / m_partitionWidth);
	int trY = std::floor((position.y + radius - m_bottomLeft.y) / m_partitionWidth);
	
	//Concatenate OOB regions
	if (blX < 0 || blY < 0)
	{
		oob = true;
		blX = std::max(blX, 0);
		blY = std::max(blY, 0);
	}
	if (trX >= m_sizeX || trY >= m_sizeY)
	{
		oob = true;
		trX = std::min(trX, m_sizeX);
		trY = std::min(trY, m_sizeY);
	}

	//Catch inversions
	blX = std::min(blX, trX);
	blY = std::min(blY, trY);
	trX = std::max(trX, blX);
	trY = std::max(trY, blY);

	return CellRange(blX, blY, trX, trY, oob);
}

void SpacePartition::addActor(const Boid* boid)
{
	if (!boid)
		return;

	vec3 position = boid->getPosition();

	getCell(position).actors.push_back(boid);
}

void SpacePartition::removeActor(const Boid* boid)
{
	if (!boid)
		return;

	vec3 position = boid->getPosition();

	getCell(position).actors.remove(boid);
}

void SpacePartition::addObstacle(vec3 obstacle)
{
	getCell(obstacle).obstacles.push_back(obstacle);
}

void SpacePartition::removeObstacle(vec3 obstacle)
{
	getCell(obstacle).obstacles.remove(obstacle);
}

void SpacePartition::haveMoved(const Boid* boid, vec3 oldPosition)
{
	if (!boid)
		return;

	vec3 position = boid->getPosition();

	std::list<const Boid*>& newCell = getCell(position).actors;
	std::list<const Boid*>& oldCell = getCell(oldPosition).actors;

	if (newCell == oldCell)
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
	m_partitions.resize(sizeX * sizeY);
}

SpacePartition::~SpacePartition()
{
}
