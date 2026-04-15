//Author: DOYEONG LEE
//Project: CubeEngine
//File: Physics2D.hpp
#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Interface/IComponent.hpp"
#include "CollisionMode.hpp"
#ifdef _DEBUG
#include "BasicComponents/DynamicSprite.hpp"
#endif

#ifdef _DEBUG
struct Point
{
	glm::vec2 pos = { 0.f,0.f };
	DynamicSprite* sprite = nullptr;
};
#endif

enum class BodyType
{
	RIGID = 0,
	BLOCK = 1
};

enum class CollideType
{
	CIRCLE = 0,
	POLYGON = 1,
	NONE = 2
};

struct Circle
{
	float radius = 0;
};

class Physics2D : public IComponent
{
public:
	Physics2D() : IComponent(ComponentTypes::PHYSICS2D) { Init(); };
	~Physics2D() override;

	void Init() override ;
	void Update(float dt) override;
	void End() override {};

	void SetVelocity(glm::vec2 v) { velocity = v; }
	void SetVelocityX(float v) { velocity.x = v; }
	void SetVelocityY(float v) { velocity.y = v; }

	void Gravity(float dt);
	void SetMinVelocity(glm::vec2 v) { velocityMin = v; }
	void SetMaxVelocity(glm::vec2 v) { velocityMax = v; }

	glm::vec2 GetVelocity() { return velocity; }

	void SetAcceleration(glm::vec2 v) { acceleration = v; };
	void AddForce(glm::vec2 v) { force = v; }
	void AddForceX(float amount) { force.x = amount; }
	void AddForceY(float amount) { force.y = amount; }

	glm::vec2 GetAngularVelocity() const { return angularVelocity; }
	float GetMomentOfInertia() const { return momentOfInertia; }
	float GetInverseInertia() const { return inverseInertia; }

	void SetAngularVelocity(glm::vec2 v) { angularVelocity = v; }
	void AddTorque(glm::vec2 t) { torque += t; }
	void SetMomentOfInertia(float i);

	void SetFriction(float f) { friction = f; }
	void SetGravity(float g, bool isGravityOnParam = true) { gravity = g; isGravityOn = isGravityOnParam; }
	void SetIsGravityOn(bool isGravityOnParam) {isGravityOn = isGravityOnParam; }
	void SetMass(float amount) { mass = amount; }
	void SetRestitution(float amount) { restitution = amount; }
	void SetBodyType(BodyType type) { bodyType = type; };
	void SetIsGhostCollision(bool state) { isGhostCollision = state; }

	// Getters
	BodyType GetBodyType() { return bodyType; };
	CollideType GetCollideType() { return collideType; };
	float GetCircleCollideRadius() { return circle.radius; }
	std::vector<glm::vec2> GetCollidePolygon() { return collidePolygon; }
	float GetRestitution() { return restitution; }
	bool GetIsGravityOn() const { return isGravityOn; }
	bool GetIsGhostCollision() { return isGhostCollision; }
	bool GetEnableRotationalPhysics() const { return enableRotationalPhysics; }

	void SetEnableRotationalPhysics(bool v) { enableRotationalPhysics = v; }

	float GetGravity() const { return gravity; }
	float GetFriction() const { return friction; }
	float GetMass() const { return mass; }
	glm::vec2 GetMinVelocity() const { return velocityMin; }
	glm::vec2 GetMaxVelocity() const { return velocityMax; }


	bool CheckCollision(Object* obj);
	bool CollisionPP(Object* obj, Object* obj2, CollisionMode mode = static_cast<CollisionMode>(0));
	bool CollisionCC(Object* obj, Object* obj2, CollisionMode mode = static_cast<CollisionMode>(0));
	bool CollisionPC(Object* poly, Object* cir, CollisionMode mode = static_cast<CollisionMode>(0));
	bool CollisionPPWithoutPhysics(Object* obj, Object* obj2);

	void AddCollideCircle(float r);
	void AddCollidePolygon(glm::vec2 position);
	void AddCollidePolygonAABB(glm::vec2 min, glm::vec2 max);
	void AddCollidePolygonAABB(glm::vec2 size);
private:
	// Mathematical helper methods for collision detection
	glm::vec2 FindSATCenter(const std::vector<glm::vec2>& points);
	glm::vec2 FindClosestPointOnSegment(const glm::vec2& circleCenter, std::vector<glm::vec2>& vertices);
	float calculatePolygonRadius(const std::vector<glm::vec2>& vertices);

	float DegreesToRadians(float degrees);
	glm::vec2 RotatePoint(const glm::vec2 point, const glm::vec2 size, float angle);
	
	// SAT (Separating Axis Theorem) helper functions
	bool IsSeparatingAxis(const glm::vec2 axis, const std::vector<glm::vec2> points1, const std::vector<glm::vec2> points2, float* axisDepth, float* min1, float* max1, float* min2, float* max2);
	bool IsSeparatingAxis(const glm::vec2 axis, const std::vector<glm::vec2> pointsPoly, const glm::vec2 pointCir, const float radius, float* axisDepth, float* min1, float* max1, float* min2, float* max2);
	
	// Calculates and applies impulses due to collision
	void CalculateLinearVelocity(Physics2D& body, Physics2D& body2, glm::vec2 normal, float* axisDepth, glm::vec2 contactPoint);

	float Length(glm::vec2 a)
	{
		return static_cast<float>(sqrt((a.x) * (a.x) + (a.y) * (a.y)));
	}
	float Distance(glm::vec2 a, glm::vec2 b)
	{
		return static_cast<float>(sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)));
	}

	float LengthSquared(glm::vec2 a)
	{
		return (a.x * a.x) + (a.y * a.y);
	}
	float DistanceSquared(glm::vec2 a, glm::vec2 b)
	{
		return ((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
	}
	
	Circle circle;
	std::vector<glm::vec2> collidePolygon;

	// Physical properties of the 2D body
	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 velocityMin = { 0.f, 0.f };
	glm::vec2 velocityMax = { 4.f, 4.f };

	glm::vec2 acceleration = { 0.f, 0.f };
	glm::vec2 force = { 0.f,0.f };
	float friction = 0.9f;
	float gravity = 9.8f;
	float mass = 1.f;
	float restitution = 0.f; // Bounciness factor

	glm::vec2 angularVelocity = { 0.f, 0.f };
	glm::vec2 torque = { 0.f, 0.f };
	float momentOfInertia = 1.0f;
	float inverseInertia = 1.0f;

	CollideType collideType = CollideType::NONE;
	BodyType bodyType = BodyType::RIGID;
	bool isGhostCollision = false;
	bool isGravityOn = false;
	bool enableRotationalPhysics = false;
#ifdef _DEBUG
	void AddPoint(glm::vec2 pos);
	std::vector<Point> points;
#endif
};