#include "Shape.h"
#define _USE_MATH_DEFINES
#include <math.h>

bool Shape::isPointInside(vec3 point)
{
	return false;//TODO
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
			float angle = atan(diff.y / diff.x);
		}
		m_lines.push_back(Line(lastPoint, angle));
		lastPoint = point;
	}
	m_lines.sort();
}

void Shape::addSquare(vec3 dir, float length)
{
	dir = dir.unit() * length;
	std::list<vec3> squarePoints;
	squarePoints.push_back(dir);
	squarePoints.push_back(vec3(dir.y, -dir.x, 0.0f));
	squarePoints.push_back(vec3()-dir);
	squarePoints.push_back(vec3(-dir.y, dir.x, 0.0f));
	Shape tempShape = //TODO
}

void Shape::addCone(vec3 dir, float width, float length)
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
