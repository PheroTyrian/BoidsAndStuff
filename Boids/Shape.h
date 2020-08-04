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

	Shape(vec3 position, std::list<Line>& lines);
	//Creates a square
	Shape(vec3 position, float dir, float length);
	//Creates a cone
	Shape(vec3 position, float dir, float width, float length);
};

