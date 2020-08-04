#include "Shape.h"

bool Shape::isPointInside(vec3 point)
{
	return false;
}

Shape::Shape(vec3 position, std::list<Line>& lines)
{
}

Shape::Shape(BasicShape type, vec3 position, float angle, float length)
{
}

vec3 Shape::Line::getFacingVec()
{
	return vec3();
}
