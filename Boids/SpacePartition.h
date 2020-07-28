#pragma once

#include <vector>
#include <list>
#include "vec3.h"

class Boid;

struct CellRange
{
	int m_blX, m_blY;
	int m_trX, m_trY;
	bool m_incOOB;

	CellRange(int blX, int blY, int trX, int trY, int oob) :
		m_blX(blX), m_blY(blY), m_trX(trX), m_trY(trY), m_incOOB(oob) {}
};

class SpacePartition
{
private:
	int m_storedObjects;
	int m_sizeX, m_sizeY;
	float m_partitionWidth;
	vec3 m_bottomLeft;
	vec3 m_topRight;
	std::vector<std::list<const Boid*>> m_partitions;
	std::list<const Boid*> m_oob;

public:
	bool isOutOfBounds(int x, int y);
	bool isOutOfBounds(vec3 position);

	std::list<const Boid*>& getOOB();

	std::list<const Boid*>& getCell(int x, int y);
	std::list<const Boid*>& getCell(vec3 position);

	CellRange findCellRange(vec3 position, float radius);

	void add(const Boid* boid);

	void haveMoved(const Boid* boid, vec3 oldPosition);

	SpacePartition(int sizeX, int sizeY, float partitionWidth);

	~SpacePartition();
};