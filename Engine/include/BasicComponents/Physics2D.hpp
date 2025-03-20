//Author: DOYEONG LEE
//Project: CubeEngine
//File: Physics2D.hpp
#pragma once
#include <glm/vec2.hpp>

#include "Component.hpp"
#ifdef _DEBUG
#include "Sprite.hpp"
#endif

#ifdef _DEBUG
struct Point
{
	glm::vec2 pos = { 0.f,0.f };
	Sprite* sprite = nullptr;
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

class Physics2D : public Component
{
public:
	Physics2D() : Component(ComponentTypes::PHYSICS2D) { Init(); };
	~Physics2D() override;

	void Init() override ;
	void Update(float dt) override;
	void UpdateForParticle(float dt, glm::vec3& pos);
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

	void SetFriction(float f) { friction = f; }
	void SetGravity(float g, bool isGravityOn_ = true) { gravity = g; isGravityOn = isGravityOn_; }
	void SetMass(float amount) { mass = amount; }
	void SetRestitution(float amount) { restitution = amount; }
	void SetBodyType(BodyType type) { bodyType = type; };
	void SetIsGhostCollision(bool state) { isGhostCollision = state; }

	BodyType GetBodyType() { return bodyType; };
	CollideType GetCollideType() { return collideType; };
	float GetCircleCollideRadius() { return circle.radius; }
	std::vector<glm::vec2> GetCollidePolygon() { return collidePolygon; }
	float GetRestitution() { return restitution; }
	bool GetIsGhostCollision() { return isGhostCollision; }

	bool CheckCollision(Object* obj);
	bool CollisionPP(Object* obj, Object* obj2);
	bool CollisionCC(Object* obj, Object* obj2);
	bool CollisionPC(Object* poly, Object* cir);
	bool CollisionPPWithoutPhysics(Object* obj, Object* obj2);

	void AddCollideCircle(float r);
	void AddCollidePolygon(glm::vec2 position);
	void AddCollidePolygonAABB(glm::vec2 min, glm::vec2 max);
	void AddCollidePolygonAABB(glm::vec2 size);
private:
	glm::vec2 FindSATCenter(const std::vector<glm::vec2>& points_);
	glm::vec2 FindClosestPointOnSegment(const glm::vec2& circleCenter, std::vector<glm::vec2>& vertices);
	float calculatePolygonRadius(const std::vector<glm::vec2>& vertices);

	float DegreesToRadians(float degrees);
	glm::vec2 RotatePoint(const glm::vec2 point, const glm::vec2 size, float angle);
	bool IsSeparatingAxis(const glm::vec2 axis, const std::vector<glm::vec2> points1, const std::vector<glm::vec2> points2, float* axisDepth, float* min1_, float* max1_, float* min2_, float* max2_);
	bool IsSeparatingAxis(const glm::vec2 axis, const std::vector<glm::vec2> pointsPoly, const glm::vec2 pointCir, const float radius, float* axisDepth, float* min1_, float* max1_, float* min2_, float* max2_);
	
	void CalculateLinearVelocity(Physics2D& body, Physics2D& body2, glm::vec2 normal, float* axisDepth);

	float Length(glm::vec2 a)
	{
		return static_cast<float>(sqrt((a.x) * (a.x) + (a.y) * (a.y)));
	}
	float Distance(glm::vec2 a, glm::vec2 b)
	{
		return static_cast<float>(sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)));
	}
	
	Circle circle;
	std::vector<glm::vec2> collidePolygon;

	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 velocityMin = { 0.f, 0.f };
	glm::vec2 velocityMax = { 4.f, 4.f };

	glm::vec2 acceleration = { 0.f, 0.f };
	glm::vec2 force = { 0.f,0.f };
	float friction = 0.9f;
	float gravity = 9.8f;
	float mass = 1.f;
	float restitution = 0.f;
	//float rotateVelocity = 0.f;

	CollideType collideType = CollideType::NONE;
	BodyType bodyType = BodyType::RIGID;
	bool isGhostCollision = false;
	bool isGravityOn = false;
#ifdef _DEBUG
	void AddPoint(glm::vec2 pos);
	std::vector<Point> points;
#endif
};