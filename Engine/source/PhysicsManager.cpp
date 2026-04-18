//Author: DOYEONG LEE
//Project: CubeEngine
//File: PhysicsManager.cpp

#include "PhysicsManager.hpp"
#include "Engine.hpp"

#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#include <algorithm>

void PhysicsManager::Update(float dt)
{
    // Execute both 2D and 3D physics pipelines
    UpdatePhysics2D(dt);
    UpdatePhysics3D(dt);
}

void PhysicsManager::SetCollisionMode(ObjectType typeA, ObjectType typeB, CollisionMode mode)
{
    if (typeA > typeB)
    {
        std::swap(typeA, typeB);
    }
    collisionMaskMap[{typeA, typeB}] = mode;
}

CollisionMode PhysicsManager::GetCollisionMode(ObjectType typeA, ObjectType typeB)
{
    if (typeA > typeB)
    {
        std::swap(typeA, typeB);
    }
    auto it = collisionMaskMap.find({typeA, typeB});
    if (it != collisionMaskMap.end())
    {
        return it->second;
    }
    return CollisionMode::COLLIDE;
}

void PhysicsManager::AddBody2D(Physics2D* body)
{
    // Add the body if it's not already in the list
    if (std::find(bodies2D.begin(), bodies2D.end(), body) == bodies2D.end())
    {
        bodies2D.push_back(body);
    }
}

void PhysicsManager::RemoveBody2D(Physics2D* body)
{
    // Search and erase the specified 2D body
    auto iterator = std::find(bodies2D.begin(), bodies2D.end(), body);
    if (iterator != bodies2D.end())
    {
        bodies2D.erase(iterator);
    }
}

void PhysicsManager::AddBody3D(Physics3D* body)
{
    // Add the body if it's not already in the list
    if (std::find(bodies3D.begin(), bodies3D.end(), body) == bodies3D.end())
    {
        bodies3D.push_back(body);
    }
}

void PhysicsManager::RemoveBody3D(Physics3D* body)
{
    // Search and erase the specified 3D body
    auto iterator = std::find(bodies3D.begin(), bodies3D.end(), body);
    if (iterator != bodies3D.end())
    {
        bodies3D.erase(iterator);
    }
}

void PhysicsManager::UpdatePhysics2D(float dt)
{
    if (bodies2D.empty())
    {
        return;
    }

    // Step 1: Move objects based on their current velocity
    ApplyMovement2D(dt);

    // Step 2: Broad Phase - Quickly filter out objects that are far apart
    auto collisionPairs = BroadPhase2D();

	// Step 3: Narrow Phase - Perform detailed collision checks on potential pairs
    NarrowPhase2D(collisionPairs, dt);

    // Step 4: Notify objects about potential collisions
    for (auto& pair : collisionPairs)
    {
        Object* objectA = pair.bodyA->GetOwner();
        Object* objectB = pair.bodyB->GetOwner();

        if (objectA != nullptr && objectB != nullptr)
        {
            objectA->CollideObject(objectB);
            objectB->CollideObject(objectA);
        }
    }
}

void PhysicsManager::ApplyMovement2D(float dt)
{
    if (bodies2D.empty())
    {
        return;
    }

    // Parallel integration: each body's position update is independent
    auto handle = Engine::GetJobSystem().QueueParallelWork(
        static_cast<uint32_t>(bodies2D.size()),
        [this, dt](uint32_t begin, uint32_t end)
        {
            for (uint32_t i = begin; i < end; ++i)
            {
                Object* owner = bodies2D[i]->GetOwner();
                glm::vec2 currentVelocity = bodies2D[i]->GetVelocity();
                owner->SetXPosition(owner->GetPosition().x + currentVelocity.x * dt);
                owner->SetYPosition(owner->GetPosition().y + currentVelocity.y * dt);
            }
        },
        16
    );
    Engine::GetJobSystem().WaitForWork(handle);
}



std::vector<CollisionPair2D> PhysicsManager::BroadPhase2D()
{
    std::vector<CollisionPair2D> pairs;

    // Helper lambda to calculate the Axis-Aligned Bounding Box (AABB)
    auto getAabb2D = [](Physics2D* body, glm::vec2& outMin, glm::vec2& outMax)
    {
        Object* owner = body->GetOwner();
        glm::vec3 currentPosition = owner->GetPosition();

        if (body->GetCollideType() == CollideType::CIRCLE)
        {
            // For circles, AABB is center +/- radius
            float radius = body->GetCircleCollideRadius();
            outMin = { currentPosition.x - radius, currentPosition.y - radius };
            outMax = { currentPosition.x + radius, currentPosition.y + radius };
        }
        else
        {
            // For polygons, calculate the bounding box based on rotated vertices
            outMin = { INFINITY,  INFINITY };
            outMax = { -INFINITY, -INFINITY };

            float angleDegrees = owner->GetRotate();
            float angleRadians = angleDegrees * 3.14159265f / 180.f;
            float cosAngle = std::cos(angleRadians);
            float sinAngle = std::sin(angleRadians);

            for (const glm::vec2& point : body->GetCollidePolygon())
            {
                // Local to world rotation math
                float rotatedX = point.x * cosAngle - point.y * sinAngle;
                float rotatedY = point.x * sinAngle + point.y * cosAngle;
                glm::vec2 worldPosition = { currentPosition.x + rotatedX, currentPosition.y + rotatedY };

                outMin.x = std::min(outMin.x, worldPosition.x);
                outMin.y = std::min(outMin.y, worldPosition.y);
                outMax.x = std::max(outMax.x, worldPosition.x);
                outMax.y = std::max(outMax.y, worldPosition.y);
            }
        }
    };

    // Double-loop check for AABB overlaps
    for (size_t i = 0; i < bodies2D.size(); ++i)
    {
        glm::vec2 minA, maxA;
        getAabb2D(bodies2D[i], minA, maxA);

        for (size_t j = i + 1; j < bodies2D.size(); ++j)
        {
            glm::vec2 minB, maxB;
            getAabb2D(bodies2D[j], minB, maxB);

            // AABB Overlap test
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



void PhysicsManager::NarrowPhase2D(std::vector<CollisionPair2D>& pairs, float /*dt*/)
{
    auto it = pairs.begin();
    while (it != pairs.end())
    {
        Physics2D* bodyA = it->bodyA;
        Physics2D* bodyB = it->bodyB;
        bool isColliding = false;

        if (bodyA != nullptr && bodyB != nullptr)
        {
            Object* objectA = bodyA->GetOwner();
            Object* objectB = bodyB->GetOwner();

            if (objectA != nullptr && objectB != nullptr)
            {
                CollisionMode currentMode = GetCollisionMode(objectA->GetObjectType(), objectB->GetObjectType());
                
                if (currentMode == CollisionMode::IGNORED)
                {
                    it = pairs.erase(it);
                    continue;
                }

                CollideType typeA = bodyA->GetCollideType();
                CollideType typeB = bodyB->GetCollideType();

                // Perform SAT and distance checks, receiving results as bools.
                if (typeA == CollideType::POLYGON && typeB == CollideType::POLYGON)
                {
                    isColliding = bodyA->CollisionPP(objectA, objectB, currentMode);
                }
                else if (typeA == CollideType::CIRCLE && typeB == CollideType::CIRCLE)
                {
                    isColliding = bodyA->CollisionCC(objectA, objectB, currentMode);
                }
                else if (typeA == CollideType::POLYGON && typeB == CollideType::CIRCLE)
                {
                    isColliding = bodyA->CollisionPC(objectA, objectB, currentMode);
                }
                else if (typeA == CollideType::CIRCLE && typeB == CollideType::POLYGON)
                {
                    isColliding = bodyB->CollisionPC(objectB, objectA, currentMode);
                }
            }
        }

        // If the SAT narrow phase check determines that the objects are not actually colliding, 
        // we remove the pair from the list to eliminate false positives from the broad phase AABB check.
        if (isColliding)
        {
            ++it;
        }
        else
        {
            it = pairs.erase(it);
        }
    }
}

void PhysicsManager::UpdatePhysics3D(float dt)
{
    if (bodies3D.empty())
    {
        return;
    }

    // Step 1: Linear/Angular integration
    Integrate3D(dt);

    // Step 2: 3D Broad Phase
    auto collisionPairs = BroadPhase3D(dt);

    // Step 3: 3D Narrow Phase
    NarrowPhase3D(collisionPairs, dt);

    for (auto& pair : collisionPairs)
    {
        Object* objectA = pair.bodyA->GetOwner();
        Object* objectB = pair.bodyB->GetOwner();

        if (objectA != nullptr && objectB != nullptr)
        {
            objectA->CollideObject(objectB);
            objectB->CollideObject(objectA);
        }
    }
}

void PhysicsManager::Integrate3D(float dt)
{
    if (bodies3D.empty())
    {
        return;
    }

    // Parallel integration: each body's physics update is independent
    auto handle = Engine::GetJobSystem().QueueParallelWork(
        static_cast<uint32_t>(bodies3D.size()),
        [this, dt](uint32_t begin, uint32_t end)
        {
            for (uint32_t i = begin; i < end; ++i)
            {
                bodies3D[i]->UpdatePhysics(dt);
            }
        },
        16
    );
    Engine::GetJobSystem().WaitForWork(handle);
}

std::vector<CollisionPair3D> PhysicsManager::BroadPhase3D(float dt)
{
    std::vector<CollisionPair3D> pairs;

    // Helper lambda to calculate 3D AABB with support for Continuous Collision Detection (CCD)
    auto getAabb3D = [dt](Physics3D* body, glm::vec3& outMin, glm::vec3& outMax)
    {
        Object* currentOwner = body->GetOwner();
        glm::vec3 position = currentOwner->GetPosition();
        glm::vec3 velocity = body->GetVelocity();

        glm::vec3 halfExtent(0.5f);
        const auto& polyhedron = body->GetCollidePolyhedron();

        // Determine half-extents for Box or Sphere
        if (body->GetColliderType() == ColliderType3D::BOX && polyhedron.size() >= 7)
        {
            halfExtent = (polyhedron[6] - polyhedron[0]) * 0.5f;
        }
        else if (body->GetColliderType() == ColliderType3D::SPHERE)
        {
            float radius = body->GetSphereRadius() / 2.0f;
            if (radius > 0.0f)
            {
                halfExtent = glm::vec3(radius);
            }
        }

        outMin = position - halfExtent;
        outMax = position + halfExtent;

        // If using CCD, expand the AABB to include the entire movement path
        if (body->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
        {
            glm::vec3 displacement = velocity * dt;
            outMin = glm::min(outMin, outMin + displacement);
            outMax = glm::max(outMax, outMax + displacement);
        }
    };

    // Check for overlaps in 3D space (XYZ)
    for (size_t i = 0; i < bodies3D.size(); ++i)
    {
        glm::vec3 minA, maxA;
        getAabb3D(bodies3D[i], minA, maxA);

        for (size_t j = i + 1; j < bodies3D.size(); ++j)
        {
            glm::vec3 minB, maxB;
            getAabb3D(bodies3D[j], minB, maxB);

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

void PhysicsManager::NarrowPhase3D(std::vector<CollisionPair3D>& pairs, float dt)
{
    for (auto& pair : pairs)
    {
        Physics3D* bodyA = pair.bodyA;
        Physics3D* bodyB = pair.bodyB;

        if (bodyA == nullptr || bodyB == nullptr)
        {
            continue;
        }

        CollisionMode currentMode = GetCollisionMode(bodyA->GetOwner()->GetObjectType(), bodyB->GetOwner()->GetObjectType());

        if (currentMode == CollisionMode::IGNORED)
        {
            continue;
        }

        ColliderType3D typeA = bodyA->GetColliderType();
        ColliderType3D typeB = bodyB->GetColliderType();

        // Resolve continuous collision sub-steps if enabled
        if (bodyA->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
        {
            SolveContinuous(bodyA, dt);
        }
        if (bodyB->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
        {
            SolveContinuous(bodyB, dt);
        }

        // Discrete collision dispatch
        if (typeA == ColliderType3D::BOX && typeB == ColliderType3D::BOX)
        {
            bodyA->CollisionPP(bodyA->GetOwner(), bodyB->GetOwner(), currentMode);
        }
        else if (typeA == ColliderType3D::SPHERE && typeB == ColliderType3D::SPHERE)
        {
            bodyA->CollisionSS(bodyA->GetOwner(), bodyB->GetOwner(), currentMode);
        }
        else if (typeA == ColliderType3D::BOX && typeB == ColliderType3D::SPHERE)
        {
            bodyA->CollisionPS(bodyA->GetOwner(), bodyB->GetOwner(), currentMode);
        }
        else if (typeA == ColliderType3D::SPHERE && typeB == ColliderType3D::BOX)
        {
            bodyB->CollisionPS(bodyB->GetOwner(), bodyA->GetOwner(), currentMode);
        }
    }
}

void PhysicsManager::SolveContinuous(Physics3D* body, float dt)
{
    float remainingTime = dt;
    int iterationCount = 0;

    // User-defined or engine constants for safety
    const int maxIterations = 5;
    const float skinWidth = 0.005f;

    // Sub-stepping loop for CCD resolution
    while (remainingTime > 0.0f && iterationCount < maxIterations)
    {
        CollisionResult collisionResult = body->FindClosestCollision(remainingTime);

        // If no hit occurred within this frame slice
        if (collisionResult.hasCollided == false || collisionResult.timeOfImpact > 1.0f)
        {
            body->GetOwner()->SetPosition(body->GetOwner()->GetPosition() + body->GetVelocity() * remainingTime);
            break;
        }

        // Move the object precisely to the point of impact
        float moveFraction = (collisionResult.timeOfImpact > skinWidth) ? (collisionResult.timeOfImpact - skinWidth) : 0.0f;
        body->GetOwner()->SetPosition(body->GetOwner()->GetPosition() + body->GetVelocity() * remainingTime * moveFraction);

        // Resolve velocity impulse at the contact point
        Physics3D* otherBody = collisionResult.otherObject->GetComponent<Physics3D>();
        if (otherBody != nullptr)
        {
            glm::vec3 contactPoint = body->GetPosition() - collisionResult.collisionNormal * 0.1f;
            body->CalculateLinearVelocity(*body, *otherBody, collisionResult.collisionNormal, nullptr, contactPoint);
        }

        // Subtract the consumed time and loop again to handle potential secondary collisions
        remainingTime -= remainingTime * moveFraction;
        iterationCount++;
    }
}