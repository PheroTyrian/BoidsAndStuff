#include "Shape.h"
#define _USE_MATH_DEFINES
#include <math.h>

bool Shape::isPointInside(vec3 pointInRealspace)
{
	bool firstSet = true;
	bool onLeft = false;
	for (Line& line : m_lines)
	{
		vec3 relativePoint = pointInRealspace - (line.point + m_position);
		vec3 perpDir = vec3(-relativePoint.y, relativePoint.x, 0.0f);
		float delta = perpDir.dot(relativePoint);
		//For the point to be inside a convex shape it must be on the same side of all 
		//lines that represent that convex shape
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

void Shape::addConeSection(vec3 relativePos, float selfRadius, float objectRadius, float scaleFactor)
{
	float dist = relativePos.mag();
	float combinedRadius = selfRadius + objectRadius;
	//Create a pi/2 anticlockwise rotated perpendicular
	vec3 perpDir = vec3(-relativePos.y, relativePos.x, 0.0f);
	//Calculate components of close points on the cone
	vec3 distComponent = relativePos.unit() * (dist - combinedRadius);
	vec3 perpComponent = perpDir.unit() * tan(asin(combinedRadius / dist)) * (dist - combinedRadius);
	//Create the set of points for the cone section
	vec3 closePoint1 = distComponent - perpComponent;
	vec3 closePoint2 = distComponent + perpComponent;
	vec3 farPoint1 = closePoint1 * scaleFactor;
	vec3 farPoint2 = closePoint2 * scaleFactor;
	//Create a temporary shape that defines the new cone
	std::list<vec3> conePoints;
	conePoints.push_back(closePoint1);
	conePoints.push_back(farPoint1);
	conePoints.push_back(farPoint2);
	conePoints.push_back(closePoint2);
	Shape tempShape = Shape(conePoints, vec3());
	//Add to existing shape
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
