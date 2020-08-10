#pragma once

#include "vec3.h"
#include <list>

//Note: only handles convex shapes
class Shape
{
public:
	struct Line
	{
		vec3 point;
		float angle; //In radians
		float length;

		Line(vec3 pos, float dir, float dist) : point(pos), angle(dir), length(dist) {}
		vec3 getFacingVec();
		bool operator<(const Line& rhs);
	};
private:
	vec3 m_position;
	std::list<Line> m_lines;
public:
	//Checks if a point lies inside the collection 
	//of sorted lines that makes up the shape
	bool isPointInside(vec3 pointInRealspace);
	//Adds two shapes together to get the region 
	//defined by the area the two would intersect
	void minkowskySum(std::list<Line>& pointsToAdd);
	//Creates a square and adds it to the shape via Minkowsky summation
	void addSquare(vec3 dir, float length);
	//Creates a cone and adds it to the shape via Minkowsky summation
	void addConeSection(vec3 relativePos, float selfRadius, 
		float objectRadius, float scaleFactor);

	Shape(vec3 position);
	Shape(vec3 position, std::list<Line>& lines);
	Shape(std::list<vec3>& points, vec3 position);
};

