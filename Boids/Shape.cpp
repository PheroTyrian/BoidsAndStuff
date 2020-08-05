#include "Shape.h"
#define _USE_MATH_DEFINES
#include <math.h>

bool Shape::isPointInside(vec3 point)
{
	for (Line& line : m_lines)
	{

	}
}

void Shape::minkowskySum(std::list<Line>& pointsToAdd)
{
	m_lines.sort();
	pointsToAdd.sort();
	vec3 nextPosition = m_lines.front().point + pointsToAdd.front().point;
	m_lines.merge(pointsToAdd);
	//Recalculate points
	for (Line& line : m_lines)
	{
		line.point = nextPosition;
		nextPosition = nextPosition + (line.getFacingVec() * line.length);
	}
}

Shape::Shape(vec3 position) : m_position(position)
{
}

Shape::Shape(vec3 position, std::list<Line>& lines) : m_position(position)
{
	for (Line line : lines)
	{
		m_lines.push_back(line);
	}
	m_lines.sort();
}

Shape::Shape(std::list<vec3>& points, vec3 position) : m_position(position)
{
	vec3 lastPoint = points.back();
	for (vec3 point : points)
	{
		vec3 diff = (point - lastPoint).unit();
		float length = (point - lastPoint).mag();
		float angle;
		if (diff.x == 0.0f)
		{
			if (diff.y > 0)
				angle = M_PI / 2;
			else
				angle = M_PI;
		}
		else
		{
			angle = atan(diff.y / diff.x);
		}
		m_lines.push_back(Line(lastPoint, angle, length));
		lastPoint = point;
	}
	m_lines.sort();
}

void Shape::addSquare(vec3 dir, float length)
{
	dir = dir.unit() * length;
	std::list<vec3> squarePoints;
	squarePoints.push_back(vec3(dir.x, dir.y, 0.0f));
	squarePoints.push_back(vec3(dir.y, -dir.x, 0.0f));
	squarePoints.push_back(vec3(-dir.x, -dir.y, 0.0f));
	squarePoints.push_back(vec3(-dir.y, dir.x, 0.0f));
	Shape tempShape = Shape(squarePoints, m_position);
	minkowskySum(tempShape.m_lines);
}

void Shape::addCone(vec3 dir, float angle, float length)
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
