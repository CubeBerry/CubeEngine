//Author: DOYEONG LEE
//Project: CubeEngine
//File: PhysicsManager.cpp

#include "PhysicsManager.hpp"
#include "Engine.hpp"

#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#include <algorithm>

// Update all physics bodies
void PhysicsManager::Update(float dt)
{
	UpdatePhysics2D(dt);
	UpdatePhysics3D(dt);
}

// Add 2D physics body to the list
void PhysicsManager::AddBody2D(Physics2D* body)
{
	if (std::find(bodies2D.begin(), bodies2D.end(), body) == bodies2D.end())
	{
		bodies2D.push_back(body);
	}
}

// Remove 2D physics body from the list
void PhysicsManager::RemoveBody2D(Physics2D* body)
{
    auto it = std::find(bodies2D.begin(), bodies2D.end(), body);
    if (it != bodies2D.end())
    {
        bodies2D.erase(it);
    }
}

// Add 3D physics body to the list
void PhysicsManager::AddBody3D(Physics3D* body)
{
    if (std::find(bodies3D.begin(), bodies3D.end(), body) == bodies3D.end())
    {
        bodies3D.push_back(body);
    }
}

// Remove 3D physics body from the list
void PhysicsManager::RemoveBody3D(Physics3D* body)
{
    auto it = std::find(bodies3D.begin(), bodies3D.end(), body);
    if (it != bodies3D.end())
    {
        bodies3D.erase(it);
    }
}

// Handle 2D physics pipeline
void PhysicsManager::UpdatePhysics2D(float dt)
{
	if (bodies2D.empty())
	{
		return;
	}

	ApplyMovement2D(dt);

	auto pairs = BroadPhase2D();

	for (auto& pair : pairs)
	{
		Object* objA = pair.bodyA->GetOwner();
		Object* objB = pair.bodyB->GetOwner();
		if (!objA || !objB)
		{
			continue;
		}
		objA->CollideObject(objB);
		objB->CollideObject(objA);
	}
}

// Apply velocity to 2D positions
void PhysicsManager::ApplyMovement2D(float dt)
{
	for (Physics2D* body : bodies2D)
	{
		Object* owner = body->GetOwner();
		glm::vec2 vel = body->GetVelocity();
		owner->SetXPosition(owner->GetPosition().x + vel.x * dt);
		owner->SetYPosition(owner->GetPosition().y + vel.y * dt);
	}
}

// Broad phase AABB filtering for 2D bodies
std::vector<CollisionPair2D> PhysicsManager::BroadPhase2D()
{
	std::vector<CollisionPair2D> pairs;

	auto getAABB2D = [](Physics2D* body, glm::vec2& outMin, glm::vec2& outMax)
	{
		Object* owner = body->GetOwner();
		glm::vec3 pos = owner->GetPosition();

		if (body->GetCollideType() == CollideType::CIRCLE)
		{
			float r = body->GetCircleCollideRadius();
			outMin = { pos.x - r, pos.y - r };
			outMax = { pos.x + r, pos.y + r };
		}
		else
		{
			outMin = {  INFINITY,  INFINITY };
			outMax = { -INFINITY, -INFINITY };

			float angleDeg = owner->GetRotate();
			float angleRad = angleDeg * 3.14159265f / 180.f;
			float cosA = std::cos(angleRad);
			float sinA = std::sin(angleRad);

			for (const glm::vec2& pt : body->GetCollidePolygon())
			{
				float rx = pt.x * cosA - pt.y * sinA;
				float ry = pt.x * sinA + pt.y * cosA;
				glm::vec2 world = { pos.x + rx, pos.y + ry };

				outMin.x = std::min(outMin.x, world.x);
				outMin.y = std::min(outMin.y, world.y);
				outMax.x = std::max(outMax.x, world.x);
				outMax.y = std::max(outMax.y, world.y);
			}
		}
	};

	for (size_t i = 0; i < bodies2D.size(); ++i)
	{
		glm::vec2 minA, maxA;
		getAABB2D(bodies2D[i], minA, maxA);

		for (size_t j = i + 1; j < bodies2D.size(); ++j)
		{
			glm::vec2 minB, maxB;
			getAABB2D(bodies2D[j], minB, maxB);

			if (maxA.x < minB.x || maxB.x < minA.x)
			{
				continue;
			}
			if (maxA.y < minB.y || maxB.y < minA.y)
			{
				continue;
			}

			pairs.push_back({ bodies2D[i], bodies2D[j] });
		}
	}
	return pairs;
}

// Exact collision tests
void PhysicsManager::NarrowPhase2D(std::vector<CollisionPair2D>& pairs, float /*dt*/)
{
	for (auto& pair : pairs)
	{
		Physics2D* a = pair.bodyA;
		Physics2D* b = pair.bodyB;
		if (!a || !b)
		{
			continue;
		}

		Object* objA = a->GetOwner();
		Object* objB = b->GetOwner();
		if (!objA || !objB)
		{
			continue;
		}

		// Dispatch by collider-type combination (reuse existing functions).
		CollideType typeA = a->GetCollideType();
		CollideType typeB = b->GetCollideType();

		if (typeA == CollideType::POLYGON && typeB == CollideType::POLYGON)
		{
			a->CollisionPP(objA, objB);
		}
		else if (typeA == CollideType::CIRCLE && typeB == CollideType::CIRCLE)
		{
			a->CollisionCC(objA, objB);
		}
		else if (typeA == CollideType::POLYGON && typeB == CollideType::CIRCLE)
		{
			a->CollisionPC(objA, objB);
		}
		else if (typeA == CollideType::CIRCLE && typeB == CollideType::POLYGON)
		{
			b->CollisionPC(objB, objA);
		}
	}
}

// Update 3D physics
void PhysicsManager::UpdatePhysics3D(float dt)
{
	if (bodies3D.empty())
	{
		return;
	}

	Integrate3D(dt);
	auto pairs = BroadPhase3D(dt);
	NarrowPhase3D(pairs, dt);

	for (auto& pair : pairs)
	{
		Object* objA = pair.bodyA->GetOwner();
		Object* objB = pair.bodyB->GetOwner();
		if (!objA || !objB)
		{
			continue;
		}
		objA->CollideObject(objB);
		objB->CollideObject(objA);
	}
}

// Integrate 3D velocities and move objects
void PhysicsManager::Integrate3D(float dt)
{
	for (Physics3D* body : bodies3D)
	{
		body->UpdatePhysics(dt);
	}
}

// Broad phase AABB filtering for 3D bodies
std::vector<CollisionPair3D> PhysicsManager::BroadPhase3D(float dt)
{
	std::vector<CollisionPair3D> pairs;

	auto getAABB3D = [dt](Physics3D* body, glm::vec3& outMin, glm::vec3& outMax)
	{
		Object* obj = body->GetOwner();
		glm::vec3 pos = obj->GetPosition();
		glm::vec3 vel = body->GetVelocity();

		glm::vec3 halfExt(0.5f);

		const auto& poly = body->GetCollidePolyhedron();
		if (body->GetColliderType() == ColliderType3D::BOX && poly.size() >= 7)
		{
			halfExt = (poly[6] - poly[0]) * 0.5f;
		}
		else if (body->GetColliderType() == ColliderType3D::SPHERE)
		{
			float r = body->GetSphereRadius() / 2.0f;
			if (r > 0.0f)
			{
				halfExt = glm::vec3(r);
			}
		}

		outMin = pos - halfExt;
		outMax = pos + halfExt;

		if (body->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
		{
			glm::vec3 disp = vel * dt;
			outMin = glm::min(outMin, outMin + disp);
			outMax = glm::max(outMax, outMax + disp);
		}
	};


	for (size_t i = 0; i < bodies3D.size(); ++i)
	{
		glm::vec3 minA, maxA;
		getAABB3D(bodies3D[i], minA, maxA);

		for (size_t j = i + 1; j < bodies3D.size(); ++j)
		{
			glm::vec3 minB, maxB;
			getAABB3D(bodies3D[j], minB, maxB);

			if (maxA.x < minB.x || maxB.x < minA.x)
			{
				continue;
			}
			if (maxA.y < minB.y || maxB.y < minA.y)
			{
				continue;
			}
			if (maxA.z < minB.z || maxB.z < minA.z)
			{
				continue;
			}

			pairs.push_back({ bodies3D[i], bodies3D[j] });
		}
	}
	return pairs;
}

// Narrow phase collision resolution for 3D bodies
void PhysicsManager::NarrowPhase3D(std::vector<CollisionPair3D>& pairs, float dt)
{
	for (auto& pair : pairs)
	{
		Physics3D* a = pair.bodyA;
		Physics3D* b = pair.bodyB;
		if (!a || !b)
		{
			continue;
		}

		ColliderType3D typeA = a->GetColliderType();
		ColliderType3D typeB = b->GetColliderType();

		bool useCCD = (a->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS ||
		               b->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS);

		if (useCCD)
		{
			if (a->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
			{
				SolveContinuous(a, dt);
			}
			if (b->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
			{
				SolveContinuous(b, dt);
			}
		}
		else
		{
			if (typeA == ColliderType3D::BOX && typeB == ColliderType3D::BOX)
			{
				SolveDiscrete_BoxBox(a, b);
			}
			else if (typeA == ColliderType3D::SPHERE && typeB == ColliderType3D::SPHERE)
			{
				SolveDiscrete_SphereSphere(a, b);
			}
			else if (typeA == ColliderType3D::BOX && typeB == ColliderType3D::SPHERE)
			{
				SolveDiscrete_BoxSphere(a, b);
			}
			else if (typeA == ColliderType3D::SPHERE && typeB == ColliderType3D::BOX)
			{
				SolveDiscrete_BoxSphere(b, a);
			}
		}
	}
}

// Resolve discrete box-box collision
void PhysicsManager::SolveDiscrete_BoxBox(Physics3D* a, Physics3D* b)
{
	a->CollisionPP(a->GetOwner(), b->GetOwner());
}

// Resolve discrete sphere-sphere collision
void PhysicsManager::SolveDiscrete_SphereSphere(Physics3D* a, Physics3D* b)
{
	a->CollisionSS(a->GetOwner(), b->GetOwner());
}

// Resolve discrete box-sphere collision
void PhysicsManager::SolveDiscrete_BoxSphere(Physics3D* box, Physics3D* sphere)
{
	box->CollisionPS(box->GetOwner(), sphere->GetOwner());
}

// Sub-step loop for continuous collision detection
void PhysicsManager::SolveContinuous(Physics3D* body, float dt)
{
	float remaining = dt;
	int   iterations = 0;

	while (remaining > 0.0f && iterations < MAX_CCD_ITERATIONS)
	{
		CollisionResult result = body->FindClosestCollision(remaining);

		if (!result.hasCollided || result.timeOfImpact > 1.0f)
		{
			body->GetOwner()->SetPosition(
				body->GetOwner()->GetPosition() + body->GetVelocity() * remaining);
			break;
		}

		float moveFraction = result.timeOfImpact > SKIN_WIDTH
			? result.timeOfImpact - SKIN_WIDTH
			: 0.0f;

		body->GetOwner()->SetPosition(
			body->GetOwner()->GetPosition() + body->GetVelocity() * remaining * moveFraction);

		Physics3D* other = result.otherObject->GetComponent<Physics3D>();
		if (other)
		{
			body->CalculateLinearVelocity(*body, *other, result.collisionNormal, nullptr);
		}

		remaining -= remaining * moveFraction;
		++iterations;
	}
}
