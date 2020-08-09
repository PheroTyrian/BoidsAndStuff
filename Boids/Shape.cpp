#include "Shape.h"
#define _USE_MATH_DEFINES
#include <math.h>

bool Shape::isPointInside(vec3 point)
{
	for (Line& line : m_lines)
	{
		bool firstSet = true;
		bool onLeft = false;
		vec3 relativePoint = point - line.point;
		double angle = atan2(relativePoint.y, relativePoint.x);
		double delta = atan2(sin(angle - (double)line.angle), cos(angle - (double)line.angle));
		if (delta > 0.0f)
		{
			if (firstSet)
				onLeft = true;
			else if (!onLeft)
				return false;
		}
		else
		{
			if (firstSet)
				onLeft = false;
			else if (onLeft)
				return false;
		}
	}
	return true;
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

void Shape::addCone(vec3 relativePos, float objectRadius, float scaleFactor)
{
	double angleFromPos = asin(objectRadius / relativePos.mag());
	double middleAngle = atan2(relativePos.y, relativePos.x);
	float angle1 = middleAngle - angleFromPos;
	float angle2 = middleAngle + angleFromPos;
	//Normalise angles
	while (angle1 < 0.0f)
		angle1 += M_PI * 2;
	while (angle2 < 0.0f)
		angle2 += M_PI * 2;
	//Get points from those angles
	vec3 point1 = vec3(cos(angle1), sin(angle1), 0.0f) * relativePos.mag() * scaleFactor;
	vec3 point2 = vec3(cos(angle2), sin(angle2), 0.0f) * relativePos.mag() * scaleFactor;
	std::list<vec3> conePoints;
	conePoints.push_back(vec3());
	conePoints.push_back(point1);
	conePoints.push_back(point2);
	Shape tempShape = Shape(conePoints, vec3());
	minkowskySum(tempShape.m_lines);
}

Shape::Shape(vec3 position) : m_position(position)
{
}

Shape::Shape(vec3 position, std::list<Line>& lines) : m_position(position)
{
	for (Line line : lines)
	{
		m_lines.emplace_back(line);
	}
	m_lines.sort();
}

Shape::Shape(std::list<vec3>& points, vec3 position) : m_position(position)
{
	vec3 lastPoint = points.back();
	for (vec3 point : points)
	{
		//Add the previous point to the shape
		vec3 diff = (point - lastPoint).unit();
		float length = (point - lastPoint).mag();
		float angle = atan2(diff.y, diff.x);
		m_lines.emplace_back(Line(lastPoint, angle, length));
		lastPoint = point;
	}
	m_lines.sort();
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
