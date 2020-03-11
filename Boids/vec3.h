#pragma once
class vec3
{
public:
	float x, y, z;

	vec3();
	vec3(float a, float b, float c);
	~vec3();

	float square() const;
	float mag() const;
	float dot(const vec3& other) const;
	vec3 unit() const;

	bool operator==(const vec3& rhs) const;
	bool operator!=(const vec3& rhs) const;
	vec3 operator+(const vec3& rhs) const;
	vec3& operator+=(const vec3& rhs);
	vec3 operator-(const vec3& rhs) const;
	vec3& operator-=(const vec3& rhs);
	vec3 operator*(const vec3& rhs) const;
	vec3 operator*(const float& rhs) const;
	vec3 operator/(const vec3& rhs) const;
	vec3 operator/(const float& rhs) const;
};

