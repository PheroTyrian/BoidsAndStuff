#pragma once
#include "vec3.h"
#include "Renderer.h"
#include "Texture.h"
#include "glm/glm.hpp"
#include <vector>

class Boid
{
private:
	vec3 m_position;
	vec3 m_velocity;
	vec3 m_acceleration;
	float m_maxAcceleration;
	float m_maxSpeed;
	float m_homeDist;
	float m_viewArc;
	float m_avoidanceDistance;
	float m_detectionDistance;
	float m_damping = true;

	VertexArray& m_vao;
	IndexBuffer& m_ib;
	Texture& m_tex;
	Shader& m_shader;
public:
	Boid(vec3 pos, vec3 vel, float maxAcc, float speed, float home, float viewArc, float avoid, float detect, VertexArray& vao, IndexBuffer& ia, Texture& tex, Shader& shader);
	~Boid();

	void update(std::vector<Boid>& boids, std::vector<vec3>& obstacle);
	void simulate(float deltaT);
	void draw(Renderer& renderer, glm::mat4 viewProjection);

	vec3 getPosition() { return m_position; }
	vec3 getVelocity() { return m_velocity; }
	void setDamping(bool damp) { m_damping = damp; }
	void setMaxAcceleration(float newMax) { m_maxAcceleration = newMax; }
	void setSpeed(float newSpeed) { m_maxSpeed = newSpeed; }
	void setHomeDist(float newDist) { m_homeDist = newDist; }
	void setAvoidanceDist(float newDist) { m_avoidanceDistance = newDist; }
	void setDetectionDist(float newDist) { m_detectionDistance = newDist; }
};
