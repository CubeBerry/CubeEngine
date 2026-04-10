//Author: DOYEONG LEE
//Project: CubeEngine
//File: Physics3D.hpp

#pragma once
#include <math.h>
#include <glm/vec3.hpp>
#pragma warning(disable : 4201)
#include <glm/gtc/quaternion.hpp>
#pragma warning(default : 4201)

#include "Interface/IComponent.hpp"
#include "Object.hpp"

enum class CollisionDetectionMode
{
	DISCRETE,
	CONTINUOUS
};

struct CollisionResult
{
	bool hasCollided = false;
	float timeOfImpact = 1.0f;
	Object* otherObject = nullptr;
	glm::vec3 collisionNormal = { 0.f, 0.f, 0.f };
};



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

class Physics3D : public IComponent
{
public:
	Physics3D() : IComponent(ComponentTypes::PHYSICS3D) { Init(); };
	~Physics3D() override;

	void Init() override ;
	void Update(float dt) override;
	void UpdatePhysics(float dt);
	void End() override {};

	void SetVelocity(glm::vec3 v) { velocity = v; }
	void SetVelocityX(float v) { velocity.x = v; }
	void SetVelocityY(float v) { velocity.y = v; }
	void SetVelocityZ(float v) { velocity.z = v; }

	void Gravity(float dt);
	void SetMinVelocity(glm::vec3 v) { velocityMin = v; }
	void SetMaxVelocity(glm::vec3 v) { velocityMax = v; }

	glm::vec3 GetVelocity() const { return velocity; }
	glm::vec3 GetAngularVelocity() const { return angularVelocity; }
	float GetMomentOfInertia() const { return momentOfInertia; }
	float GetInverseInertia() const { return inverseInertia; }

	void SetAngularVelocity(glm::vec3 v) { angularVelocity = v; }
	void AddTorque(glm::vec3 t) { torque += t; }
	void SetMomentOfInertia(float i);

	glm::vec3 GetMinVelocity() const { return velocityMin; }
	glm::vec3 GetMaxVelocity() const { return velocityMax; }
	float GetGravity() const { return gravity; }
	bool GetIsGravityOn() const { return isGravityOn; }
	float GetFriction() const { return friction; }
	float GetMass() const { return mass; }
	glm::quat GetOrientation() const { return orientation; }
	glm::vec3 GetPosition() const { return GetOwner()->GetPosition(); }
	float GetRestitution() const { return restitution; }

	void Awake();
	void SetAcceleration(glm::vec3 v);
	void AddForce(glm::vec3 v);
	void AddForceX(float amount);
	void AddForceY(float amount);
	void AddForceZ(float amount);
	void Teleport(glm::vec3 newPosition);

	void SetFriction(float f) { friction = f; }
	void SetGravity(float g, bool isGravityOnParam = true) 
	{ 
		gravity = g; 
		isGravityOn = isGravityOnParam; 
		if (isGravityOnParam) Awake(); 
	}
	void SetIsGravityOn(bool state) 
	{ 
		isGravityOn = state; 
		if (state) Awake(); 
	}
	void SetMass(float m);
	void SetRestitution(float amount) { restitution = amount; }

	void SetOrientation(glm::quat q) { orientation = q; }

	bool GetIsGhostCollision() const { return isGhostCollision; }
	BodyType3D GetBodyType() const { return bodyType; };
	ColliderType3D GetColliderType() const { return colliderType; }
	CollisionDetectionMode GetCollisionDetectionMode() const { return collisionMode; }

	void SetColliderType(ColliderType3D type) { colliderType = type; }
	void SetIsGhostCollision(bool state) { isGhostCollision = state; }
	void SetBodyType(BodyType3D type) { bodyType = type; };
	void SetCollisionDetectionMode(CollisionDetectionMode mode) { collisionMode = mode; }
	bool GetEnableRotationalPhysics() const { return enableRotationalPhysics; }
	void SetEnableRotationalPhysics(bool v);

	//2d->3d
	std::vector<glm::vec3> GetCollidePolyhedron() { return collidePolyhedron; }
	float GetSphereRadius() const { return sphere.radius; }
	bool CheckCollision(Object* obj);
	bool CollisionPP(Object* obj, Object* obj2);
	bool CollisionSS(Object* obj, Object* obj2);
	bool CollisionPS(Object* poly, Object* sph);

	void AddCollidePolyhedron(glm::vec3 position);
	void AddCollidePolyhedronAABB(glm::vec3 min, glm::vec3 max);
	void AddCollidePolyhedronAABB(glm::vec3 size);
	void AddCollideSphere(float r);
	//2d->3d

	// Made public so PhysicsManager can drive the CCD loop directly.
	CollisionResult FindClosestCollision(float dt);
	void CalculateLinearVelocity(Physics3D& body, Physics3D& body2, glm::vec3 normal, float* axisDepth, glm::vec3 contactPoint, float impulseScale = 1.0f);

private:
	// Linear Physical Properties
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 velocityMin = { 0.f, 0.f, 0.0f };
	glm::vec3 velocityMax = { 4.f, 4.f, 4.f };

	glm::vec3 acceleration = { 0.f, 0.f, 0.0f };
	glm::vec3 force = { 0.f,0.f, 0.0f };
	float friction = 0.9f;
	float gravity = 9.8f;
	float mass = 1.f;
	float restitution = 0.f; // Bounciness factor
	glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	// Physics States
	bool isGhostCollision = false;
	bool isGravityOn = false;
	bool enableRotationalPhysics = false;
	bool isSleeping = false;
	float sleepTimer = 0.0f;
	float angularDamping = 0.5f;

	glm::vec3 angularVelocity = { 0.f, 0.f, 0.f };
	glm::vec3 torque = { 0.f, 0.f, 0.f };
	float momentOfInertia = 1.0f;
	float inverseInertia = 1.0f;

	ColliderType3D colliderType =  ColliderType3D::BOX;
	BodyType3D bodyType = BodyType3D::RIGID;
	CollisionDetectionMode collisionMode = CollisionDetectionMode::DISCRETE;

	//2d->3d
	// Discrete Collision Helpers (SAT)
	glm::vec3 FindSATCenter(const std::vector<glm::vec3>& points);
	glm::vec3 RotatePoint(const glm::vec3& point, const glm::vec3& position, const glm::quat& rotation);
	bool IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> points1, const std::vector<glm::vec3> points2, float* axisDepth, float* min1, float* max1, float* min2, float* max2);
	
	glm::vec3 FindClosestPointOnSegment(const glm::vec3& sphereCenter, std::vector<glm::vec3>& vertices);
	bool IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> pointsPoly, const glm::vec3 pointSphere, const float radius, float* axisDepth, float* min1, float* max1, float* min2, float* max2);

	// Continuous Collision Helpers (CCD)
	void ProjectPolygon(const std::vector<glm::vec3>& vertices, const glm::vec3& axis, float& min, float& max);
	bool StaticSATIntersection(Physics3D* body1, Physics3D* body2,
		const std::vector<glm::vec3>& rotatedPoly1, const std::vector<glm::vec3>& rotatedPoly2,
		const glm::mat4& rotationMatrix1, const glm::mat4& rotationMatrix2,
		glm::vec3& outNormal, float& outDepth);
	bool SweptSATOBB(Physics3D* body1, Physics3D* body2, float dt, CollisionResult& outResult);
	bool SweptSpheres(Physics3D* body1, Physics3D* body2, float dt, CollisionResult& outResult);
	bool SweptSphereVsOBB(Physics3D* boxBody, float dt, CollisionResult& outResult);
	//CollisionDetectionMode : Continuous

	std::vector<glm::vec3> collidePolyhedron;
	Sphere sphere;

	//2d->3d

};