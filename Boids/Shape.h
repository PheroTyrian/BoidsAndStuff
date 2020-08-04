#pragma once

#include "vec3.h"
#include <list>

enum class BasicShape
{
	square,
	cone
};

//Note: only handles convex shapes
class Shape
{
public:
	struct Line
	{
		vec3 point;
		float angle; //In radians

		Line(vec3 pos, float dir) : point(pos), angle(dir) {}
		vec3 getFacingVec();
		bool operator<(const Line& rhs);
	};
private:
	vec3 m_position;
	std::list<Line> m_lines;
public:
	bool isPointInside(vec3 point);

	Shape(vec3 position);
	Shape(vec3 position, std::list<Line>& lines);
	Shape(std::list<vec3>& points, vec3 position);
	//Creates a square and adds it to the shape via 
	void addSquare(vec3 dir, float length);
	//Creates a cone
	void addCone(vec3 dir, float width, float length);
};

