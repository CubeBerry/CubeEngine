//Author: DOYEONG LEE
//Project: CubeEngine
//File: Physics3D.hpp

#pragma once
#include <math.h>
#include <glm/vec3.hpp>
#pragma warning(disable : 4201)
#include <glm/gtc/quaternion.hpp>
#pragma warning(default : 4201)

#include "Component.hpp"
#include "Object.hpp"
#ifdef _DEBUG
#include "Sprite.hpp"
#endif

#ifdef _DEBUG
struct Point3D
{
	glm::vec3 pos = { 0.f,0.f,0.f };
	Sprite* sprite = nullptr;
};
#endif

enum class BodyType3D
{
	RIGID = 0,
	BLOCK = 1
};

enum class ColliderType3D 
{
	SPHERE,
	BOX
};

struct Sphere
{
	float radius = 0;
};

class Physics3D : public Component
{
public:
	Physics3D() : Component(ComponentTypes::PHYSICS3D) {/* Init(); */};
	~Physics3D() override;

	void Init() override ;
	void Update(float dt) override;
	void UpdatePhysics(float dt);
	void UpdateForParticle(float dt, glm::vec3& pos);
	void End() override {};

	void SetVelocity(glm::vec3 v) { velocity = v; }
	void SetVelocityX(float v) { velocity.x = v; }
	void SetVelocityY(float v) { velocity.y = v; }
	void SetVelocityZ(float v) { velocity.z = v; }

	void Gravity(float dt);
	void SetMinVelocity(glm::vec3 v) { velocityMin = v; }
	void SetMaxVelocity(glm::vec3 v) { velocityMax = v; }

	glm::vec3 GetVelocity() const { return velocity; } 
	glm::vec3 GetMinVelocity() const { return velocityMin; }
	glm::vec3 GetMaxVelocity() const { return velocityMax; }
	float GetGravity() const { return gravity; }
	bool GetIsGravityOn() const { return isGravityOn; }
	float GetFriction() const { return friction; }
	float GetMass() const { return mass; }
	glm::quat GetOrientation() const { return orientation; }
	glm::vec3 GetPosition() const { return GetOwner()->GetPosition(); }
	float GetRestitution() const { return restitution; }

	void SetAcceleration(glm::vec3 v) { acceleration = v; };
	void AddForce(glm::vec3 v) { force = v; }
	void AddForceX(float amount) { force.x = amount; }
	void AddForceY(float amount) { force.y = amount; }
	void AddForceZ(float amount) { force.z = amount; }

	void SetFriction(float f) { friction = f; }
	void SetGravity(float g, bool isGravityOn_ = true) { gravity = g; isGravityOn = isGravityOn_; }
	void SetMass(float m) { mass = m; }
	void SetRestitution(float amount) { restitution = amount; }

	void SetOrientation(glm::quat q) { orientation = q; }

	bool GetIsGhostCollision() const { return isGhostCollision; }
	BodyType3D GetBodyType() const { return bodyType; };
	ColliderType3D GetColliderType() const { return colliderType; }

	void SetColliderType(ColliderType3D type) { colliderType = type; }
	void SetIsGhostCollision(bool state) { isGhostCollision = state; }
	void SetBodyType(BodyType3D type) { bodyType = type; };

	//2d->3d
	std::vector<glm::vec3> GetCollidePolyhedron() { return collidePolyhedron; }
	bool CheckCollision(Object* obj);
	bool CollisionPP(Object* obj, Object* obj2);
	bool CollisionSS(Object* obj, Object* obj2);
	bool CollisionPS(Object* poly, Object* sph);

	void AddCollidePolyhedron(glm::vec3 position);
	void AddCollidePolyhedronAABB(glm::vec3 min, glm::vec3 max);
	void AddCollidePolyhedronAABB(glm::vec3 size);
	void AddCollideSphere(float r);
	//2d->3d
private:
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 velocityMin = { 0.f, 0.f, 0.0f };
	glm::vec3 velocityMax = { 4.f, 4.f, 4.f };

	glm::vec3 acceleration = { 0.f, 0.f, 0.0f };
	glm::vec3 force = { 0.f,0.f, 0.0f };
	float friction = 0.9f;
	float gravity = 9.8f;
	float mass = 1.f;
	float restitution = 0.f;
	glm::quat orientation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

	bool isGhostCollision = false;
	bool isGravityOn = false;

	ColliderType3D colliderType =  ColliderType3D::BOX;
	BodyType3D bodyType = BodyType3D::RIGID;

	//2d->3d
	glm::vec3 FindSATCenter(const std::vector<glm::vec3>& points_);
	glm::vec3 RotatePoint(const glm::vec3& point, const glm::vec3& position, const glm::quat& rotation);
	bool IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> points1, const std::vector<glm::vec3> points2, float* axisDepth, float* min1_, float* max1_, float* min2_, float* max2_);
	void CalculateLinearVelocity(Physics3D& body, Physics3D& body2, glm::vec3 normal, float* axisDepth);
	
	glm::vec3 FindClosestPointOnSegment(const glm::vec3& sphereCenter, std::vector<glm::vec3>& vertices);
	bool IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> pointsPoly, const glm::vec3 pointSphere, const float radius, float* axisDepth, float* min1_, float* max1_, float* min2_, float* max2_);

	std::vector<glm::vec3> collidePolyhedron;
	Sphere sphere;

	//2d->3d
#ifdef _DEBUG
	void AddPoint(glm::vec3 pos);
	std::vector<Point3D> points;
#endif
};