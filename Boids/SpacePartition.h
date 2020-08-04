#pragma once

#include <vector>
#include <list>
#include "vec3.h"

class Boid;

struct CellRange
{
	int blX, blY;
	int trX, trY;
	bool incOOB;

	CellRange(int bl_X, int bl_Y, int tr_X, int tr_Y, int oob) :
		blX(bl_X), blY(bl_Y), trX(tr_X), trY(tr_Y), incOOB(oob) {}
};

class SpacePartition
{
private:
	struct Cell
	{
		std::list<const Boid*> actors;
		std::list<vec3> obstacles;
	};

	int m_storedObjects;
	int m_sizeX, m_sizeY;
	float m_partitionWidth;
	vec3 m_bottomLeft;
	vec3 m_topRight;
	std::vector<Cell> m_partitions;
	Cell m_oob;

public:
	bool isOutOfBounds(int x, int y) const;
	bool isOutOfBounds(vec3 position) const;

	int getStoredObjects() { return m_storedObjects; }
	int getSizeX() { return m_sizeX; }
	int getSizeY() { return m_sizeY; }
	const Cell& getOOB() const { return m_oob; }
	
	const Cell& getCell(int x, int y) const;
	Cell& getCell(vec3 position);

	CellRange findCellRange(vec3 position, float radius) const;

	void addActor(const Boid* boid);
	void removeActor(const Boid* boid);
	void addObstacle(vec3 obstacle);
	void removeObstacle(vec3 obstacle);

	void haveMoved(const Boid* boid, vec3 oldPosition);

	SpacePartition(int sizeX, int sizeY, float partitionWidth);

	~SpacePartition();
};