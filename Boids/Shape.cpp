#include "Shape.h"
#define _USE_MATH_DEFINES
#include <math.h>

bool Shape::isPointInside(vec3 point)
{
	return false;//TODO
}

Shape::Shape(vec3 position, std::list<Line>& lines) : m_position(position)
{
	for (Line line : lines)
	{
		m_lines.push_back(line);
	}
	m_lines.sort();
}

Shape::Shape(vec3 position, float dir, float length)
{
	
}

Shape::Shape(vec3 position, float dir, float width, float length)
{
}

vec3 Shape::Line::getFacingVec()
{
	return vec3(cos(angle), sin(angle), 0.0f);
}

bool Shape::Line::operator<(const Line& rhs)
{
	if (angle < rhs.angle)
		return true;
	else
		return false;
}
