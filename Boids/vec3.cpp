#include "pch.h"
#include "vec3.h"
#include <cmath>


vec3::vec3() : x(0.0f), y(0.0f), z(0.0f)
{
}

vec3::vec3(float a, float b, float c) : x(a), y(b), z(c)
{
}


vec3::~vec3()
{
}

float vec3::square() const
{
	return (x * x) + (y * y) + (z * z);
}

float vec3::mag() const
{
	return std::sqrtf(square());
}

float vec3::dot(const vec3 & other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z);
}

vec3 vec3::unit() const
{
	float m = mag();
	return vec3(x / m, y / m, z / m);
}

inline bool vec3::operator==(const vec3 & rhs) const
{
	if (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z)
		return true;
	else
		return false;
}

inline bool vec3::operator!=(const vec3 & rhs) const
{
	if (this->x != rhs.x || this->y != rhs.y || this->z != rhs.z)
		return true;
	else 
		return false;
}

inline vec3 vec3::operator+(const vec3 & rhs) const
{
	return vec3(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
}

inline vec3 & vec3::operator+=(const vec3 & rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
	return *this;
}

inline vec3 vec3::operator-(const vec3 & rhs) const
{
	return vec3(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
}

inline vec3 & vec3::operator-=(const vec3 & rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
	return *this;
}

inline vec3 vec3::operator*(const vec3 & rhs) const
{
	return vec3(this->x * rhs.x, this->y * rhs.y, this->z * rhs.z);
}

inline vec3 vec3::operator*(const float & rhs) const
{
	return vec3(this->x * rhs, this->y * rhs, this->z * rhs);
}

inline vec3 vec3::operator/(const vec3 & rhs) const
{
	return vec3(this->x / rhs.x, this->y / rhs.y, this->z / rhs.z);
}

inline vec3 vec3::operator/(const float & rhs) const
{
	return vec3(this->x / rhs, this->y / rhs, this->z / rhs);
}
