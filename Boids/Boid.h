#pragma once
#include "vec3.h"
#include "Renderer.h"
#include "Texture.h"
#include "glm/glm.hpp"
#include <vector>

class SpacePartition;

class Boid
{
private:
	vec3 m_position;
	vec3 m_velocity;
	vec3 m_acceleration;
	vec3 m_homeLocation;

	float m_maxAcceleration;
	float m_maxSpeed;
	float m_homeDist;
	float m_viewArc;
	float m_radius;
	float m_avoidanceDistance;
	float m_detectionDistance;
	bool m_damping = true;

	SpacePartition& m_partition;
	VertexArray& m_vao;
	IndexBuffer& m_ib;
	Texture& m_tex;
	Texture& m_outlineR;
	Texture& m_outlineB;
	Shader& m_shader;
public:
	Boid(vec3 pos, vec3 vel, SpacePartition& partition, VertexArray& vao, 
		IndexBuffer& ia, Texture& tex, Texture& outlineTexR, Texture& outlineTexB, 
		Shader& shader);
	~Boid();

	void steering(std::vector<Boid>& boids, std::vector<vec3>& obstacle);
	void locomotion(float deltaT);
	void draw(Renderer& renderer, glm::mat4 viewProjection);
	void drawAuras(Renderer& renderer, glm::mat4 viewProjection,
		bool drawAvoid, bool drawDetect);

	vec3 getPosition() const { return m_position; }
	vec3 getVelocity() const { return m_velocity; }
	vec3 getAcceleration() const { return m_acceleration; }
	vec3& refAcceleration() { return m_acceleration; }
	float getMaxAcceleration() const { return m_maxAcceleration; }
	float getMaxSpeed() const { return m_maxSpeed; }
	float getHomeDistance() const { return m_homeDist; }
	vec3 getHomeLocation() const { return m_homeLocation; }
	float getViewArc() const { return m_viewArc; }
	float getRadius() const { return m_radius;  }
	float getAvoidanceDist() const { return m_avoidanceDistance; }
	float getDetectionDist() const { return m_detectionDistance; }
	bool getDamping() const { return m_damping; }

	void setDamping(bool damp) { m_damping = damp; }
	void setMaxAcceleration(float newMax) { m_maxAcceleration = newMax; }
	void setSpeed(float newSpeed) { m_maxSpeed = newSpeed; }
	void setHomeDist(float newDist) { m_homeDist = newDist; }
	void setHomeLocation(vec3 newLocation) { m_homeLocation = newLocation; }
	void setViewArc(float newArc) { m_viewArc = newArc; }
	void setRadius(float newRadius) { m_radius = newRadius; }
	void setAvoidanceDist(float newDist) { m_avoidanceDistance = newDist; }
	void setDetectionDist(float newDist) { m_detectionDistance = newDist; }
};
