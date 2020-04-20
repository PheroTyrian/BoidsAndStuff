#pragma once
#include "vec3.h"
#include "Renderer.h"
#include "VertexArray.h"
#include "Texture.h"
#include <vector>

class Boid
{
private:
	vec3 m_position;
	vec3 m_velocity;
	vec3 m_acceleration;
	const float m_maxAcceleration;
	const float m_dragEffect;
	const float m_avoidanceDistance;
	const float m_detectionDistance;

	VertexArray& m_vao;
	IndexBuffer& m_ib;
	Texture& m_tex;
public:
	Boid(vec3 pos, vec3 vel, float acc, float drag, float avoid, float detect, VertexArray& vao, IndexBuffer& ia, Texture& tex);
	~Boid();

	void update(std::vector<Boid>& boids, std::vector<vec3>& obstacle);
	void simulate(float deltaT);
};

