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

	inline bool operator==(const vec3& rhs) const;
	inline bool operator!=(const vec3& rhs) const;
	inline vec3 operator+(const vec3& rhs) const;
	inline vec3& operator+=(const vec3& rhs);
	inline vec3 operator-(const vec3& rhs) const;
	inline vec3 operator*(const vec3& rhs) const;
	inline vec3 operator*(const float& rhs) const;
	inline vec3 operator/(const vec3& rhs) const;
	inline vec3 operator/(const float& rhs) const;
};

