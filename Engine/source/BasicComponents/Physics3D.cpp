//Author: DOYEONG LEE
//Project: CubeEngine
//File: Physics3D.cpp

#include "BasicComponents/Physics3D.hpp"
#include "BasicComponents/DynamicSprite.hpp"

#include "Engine.hpp"
#include <iostream>
#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <vector>
#include <cfloat>

Physics3D::~Physics3D()
{
    // Remove this body from the global physics manager upon destruction
    Engine::GetPhysicsManager().RemoveBody3D(this);
}

void Physics3D::Init()
{
    // Register this body to the physics manager for updates
    Engine::GetPhysicsManager().AddBody3D(this);
}

void Physics3D::Update(float /*dt*/)
{
    // Integration is handled globally by PhysicsManager::StepPhysics3D()
}

void Physics3D::SetMomentOfInertia(float i)
{
    // Store moment of inertia and calculate its inverse for rotation math
    momentOfInertia = i;
    if (i > 0.0f)
    {
        inverseInertia = 1.0f / i;
    }
    else
    {
        // Zero inertia represents an unrotatable object
        inverseInertia = 0.0f;
    }
}

void Physics3D::Awake()
{
    // Reset sleep state and timer to reactivate physics
    isSleeping = false;
    sleepTimer = 0.0f;
}

void Physics3D::SetEnableRotationalPhysics(bool v)
{
    enableRotationalPhysics = v;
    if (v && GetOwner() != nullptr)
    {
        // Synchronize orientation from the visual object transform
        orientation = glm::quat(-glm::radians(GetOwner()->GetRotate3D()));
        angularVelocity = glm::vec3(0.0f);
        sleepTimer = 0.0f;
    }
}

void Physics3D::SetAcceleration(glm::vec3 v)
{
    // Set direct acceleration and wake the body
    acceleration = v;
    Awake();
}

void Physics3D::AddForce(glm::vec3 v)
{
    // Accumulate force for this frame
    force += v;
    Awake();
}

void Physics3D::AddForceX(float amount)
{
    force.x += amount;
    Awake();
}

void Physics3D::AddForceY(float amount)
{
    force.y += amount;
    Awake();
}

void Physics3D::AddForceZ(float amount)
{
    force.z += amount;
    Awake();
}

void Physics3D::UpdatePhysics(float dt)
{
    if (isSleeping)
    {
        // Wake the object if any significant force or gravity is applied
        if (glm::length2(force) > 0.0001f || glm::length2(torque) > 0.0001f || glm::length2(acceleration) > 0.0001f || isGravityOn)
        {
            Awake();
        }
        else
        {
            // Keep forces zeroed out while sleeping to prevent accumulation
            force = glm::vec3(0.0f);
            torque = glm::vec3(0.0f);
            return;
        }
    }

    if (isGravityOn)
    {
        Gravity(dt);
    }

    // Apply Newton's second law: F = ma -> a = F/m
    acceleration = force / mass;
    velocity += acceleration * dt;

    if (friction > 0.f)
    {
        if (isGravityOn)
        {
            // Apply horizontal friction when gravity is active
            velocity.x *= (1.f - friction * dt);
            velocity.z *= (1.f - friction * dt);
        }
        else
        {
            // Apply full linear damping
            velocity *= (1.f - friction * dt);
        }
    }

    // Reset accumulated force after integration
    force = glm::vec3(0.0f);

    // Clamp linear velocity to the defined maximum limits
    if (std::abs(velocity.x) > velocityMax.x)
    {
        velocity.x = velocityMax.x * ((velocity.x < 0.f) ? -1.f : 1.f);
    }
    if (std::abs(velocity.y) > velocityMax.y)
    {
        velocity.y = velocityMax.y * ((velocity.y < 0.f) ? -1.f : 1.f);
    }
    if (std::abs(velocity.z) > velocityMax.z)
    {
        velocity.z = velocityMax.z * ((velocity.z < 0.f) ? -1.f : 1.f);
    }

    if (enableRotationalPhysics && inverseInertia > 0.0f)
    {
        // Calculate angular acceleration from torque
        glm::vec3 angularAcceleration = torque * inverseInertia;
        angularVelocity += angularAcceleration * dt;

        // Apply rotational damping
        angularVelocity *= (1.f - angularDamping * dt);

        // Clamp extremely low angular velocities to zero
        if (glm::length2(angularVelocity) < 1e-6f)
        {
            angularVelocity = glm::vec3(0.0f);
        }

        torque = glm::vec3(0.0f);

        // Perform quaternion integration to update orientation
        glm::quat spin = 0.5f * glm::quat(0.f, angularVelocity.x, angularVelocity.y, angularVelocity.z) * orientation;
        orientation.w += spin.w * dt;
        orientation.x += spin.x * dt;
        orientation.y += spin.y * dt;
        orientation.z += spin.z * dt;
        orientation = glm::normalize(orientation);

        // Map the orientation back to Euler angles for the visual object
        glm::vec3 eulerArgs = glm::degrees(glm::eulerAngles(orientation));
        GetOwner()->SetRotate(-eulerArgs);
    }

    // Thresholds for deciding if the object should go to sleep
    const float linearSleepThreshold = 0.01f;
    const float angularSleepThreshold = 0.0001f;

    if (glm::length2(velocity) < linearSleepThreshold && glm::length2(angularVelocity) < angularSleepThreshold)
    {
        sleepTimer += dt;
        if (sleepTimer > 0.5f)
        {
            // Object has been stationary long enough to sleep
            isSleeping = true;
            velocity = glm::vec3(0.0f);
            angularVelocity = glm::vec3(0.0f);
        }
    }
    else
    {
        sleepTimer = 0.0f;
    }

    // Zero out velocity components if they fall below the minimum threshold
    if (std::abs(velocity.x) < velocityMin.x)
    {
        velocity.x = 0.f;
    }
    if (std::abs(velocity.y) < velocityMin.y)
    {
        velocity.y = 0.f;
    }
    if (std::abs(velocity.z) < velocityMin.z)
    {
        velocity.z = 0.f;
    }

    if (collisionMode == CollisionDetectionMode::CONTINUOUS)
    {
        // Perform swept collision detection for high-speed movement
        CollisionResult collision = FindClosestCollision(dt);
        if (collision.hasCollided && collision.timeOfImpact <= 1.0f)
        {
            const float skinWidth = 0.005f;
            // Calculate time to move before impact, including a small safety skin
            float moveTime = collision.timeOfImpact > skinWidth ? collision.timeOfImpact - skinWidth : 0.0f;
            GetOwner()->SetPosition(GetOwner()->GetPosition() + velocity * dt * moveTime);

            // Calculate point of contact and resolve velocity impulse
            glm::vec3 contactPoint = GetOwner()->GetPosition() - collision.collisionNormal * 0.1f;
            CalculateLinearVelocity(*this, *collision.otherObject->GetComponent<Physics3D>(), collision.collisionNormal, nullptr, contactPoint);

            // Move the object for the remaining timeframe after resolution
            float remainingTime = dt - (dt * moveTime);
            if (remainingTime > 0.0f)
            {
                GetOwner()->SetPosition(GetOwner()->GetPosition() + velocity * remainingTime);
            }
        }
        else
        {
            GetOwner()->SetPosition(GetOwner()->GetPosition() + velocity * dt);
        }
    }
    else
    {
        // Simple discrete movement
        GetOwner()->SetPosition(GetOwner()->GetPosition() + velocity * dt);
    }
}

void Physics3D::Gravity(float dt)
{
    if (isGravityOn)
    {
        // Apply gravitational acceleration to the vertical velocity
        velocity.y -= gravity * dt;
        if (std::abs(velocity.y) > velocityMax.y)
        {
            velocity.y = velocityMax.y * ((velocity.y < 0.f) ? -1.f : 1.f);
        }
    }
}

void Physics3D::Teleport(glm::vec3 newPosition)
{
    // Instantly move the object and stop its motion
    GetOwner()->SetPosition(newPosition);
    velocity = glm::vec3(0.0f);
}

void Physics3D::SetMass(float m)
{
    if (m > 0.f)
    {
        mass = m;
    }
}

bool Physics3D::CheckCollision(Object* obj)
{
    // Dispatch to specific collision logic based on collider types
    switch (colliderType)
    {
    case ColliderType3D::BOX:
    {
        if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::BOX)
        {
            return CollisionPP(GetOwner(), obj);
        }
        else if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::SPHERE)
        {
            return CollisionPS(GetOwner(), obj);
        }
        break;
    }
    case ColliderType3D::SPHERE:
    {
        if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::BOX)
        {
            return CollisionPS(obj, GetOwner());
        }
        else if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::SPHERE)
        {
            return CollisionSS(GetOwner(), obj);
        }
        break;
    }
    default:
    {
        return false;
    }
    }
    return false;
}

bool Physics3D::CollisionPP(Object* obj, Object* obj2, CollisionMode mode)
{
    auto* physics1 = obj->GetComponent<Physics3D>();
    auto* physics2 = obj2->GetComponent<Physics3D>();

    if (physics1->GetCollidePolyhedron().empty() == false && physics2->GetCollidePolyhedron().empty() == false)
    {
        const auto& poly1 = physics1->collidePolyhedron;
        const auto& poly2 = physics2->collidePolyhedron;

        if (poly1.empty() || poly2.empty())
        {
            return false;
        }

        // Transform local polyhedron vertices to world space
        std::vector<glm::vec3> rotatedPoly1, rotatedPoly2;
        glm::quat orient1 = physics1->GetEnableRotationalPhysics() ? physics1->GetOrientation() : glm::quat(-glm::radians(obj->GetRotate3D()));
        glm::quat orient2 = physics2->GetEnableRotationalPhysics() ? physics2->GetOrientation() : glm::quat(-glm::radians(obj2->GetRotate3D()));

        glm::mat4 rotationMatrix1 = glm::mat4_cast(orient1);
        glm::mat4 rotationMatrix2 = glm::mat4_cast(orient2);

        glm::mat4 transform1 = glm::translate(glm::mat4(1.0f), obj->GetPosition()) * rotationMatrix1;
        glm::mat4 transform2 = glm::translate(glm::mat4(1.0f), obj2->GetPosition()) * rotationMatrix2;

        for (const auto& point : poly1)
        {
            rotatedPoly1.emplace_back(glm::vec3(transform1 * glm::vec4(point, 1.0f)));
        }
        for (const auto& point : poly2)
        {
            rotatedPoly2.emplace_back(glm::vec3(transform2 * glm::vec4(point, 1.0f)));
        }

        // Collect all potential separating axes (face normals and edge cross products)
        std::vector<glm::vec3> axes;
        axes.push_back(glm::normalize(glm::vec3(rotationMatrix1[0])));
        axes.push_back(glm::normalize(glm::vec3(rotationMatrix1[1])));
        axes.push_back(glm::normalize(glm::vec3(rotationMatrix1[2])));
        axes.push_back(glm::normalize(glm::vec3(rotationMatrix2[0])));
        axes.push_back(glm::normalize(glm::vec3(rotationMatrix2[1])));
        axes.push_back(glm::normalize(glm::vec3(rotationMatrix2[2])));

        for (size_t i = 0; i < rotatedPoly1.size(); ++i)
        {
            for (size_t j = 0; j < rotatedPoly2.size(); ++j)
            {
                glm::vec3 edge1 = rotatedPoly1[(i + 1) % rotatedPoly1.size()] - rotatedPoly1[i];
                glm::vec3 edge2 = rotatedPoly2[(j + 1) % rotatedPoly2.size()] - rotatedPoly2[j];
                glm::vec3 axis = glm::cross(edge1, edge2);
                if (glm::length(axis) > 0.0001f)
                {
                    axes.push_back(glm::normalize(axis));
                }
            }
        }

        float minDepth = FLT_MAX;
        glm::vec3 collisionNormal(0.f);

        // Test for separation along each axis (Separating Axis Theorem)
        for (const auto& axis : axes)
        {
            float min1, max1, min2, max2;
            float depth;
            if (IsSeparatingAxis(axis, rotatedPoly1, rotatedPoly2, &depth, &min1, &max1, &min2, &max2))
            {
                return false; // Gap found, no collision
            }
            if (depth < minDepth)
            {
                minDepth = depth;
                collisionNormal = ((max1 - min2) < (max2 - min1)) ? axis : -axis;
            }
        }

        // Resolve penetration and calculate contact physics
        if (mode == CollisionMode::COLLIDE && !physics1->GetIsGhostCollision() && !physics2->GetIsGhostCollision())
        {
            const float slop = 0.005f;
            const float correctionPercent = 0.15f;
            float penetrationAmt = std::max(minDepth - slop, 0.0f);
            glm::vec3 moveVector = collisionNormal * (penetrationAmt * correctionPercent);

            // Separate the overlapping objects based on their body type
            if (physics1->GetBodyType() == BodyType3D::RIGID && physics2->GetBodyType() == BodyType3D::RIGID)
            {
                if (glm::length2(moveVector) > 0.0f)
                {
                    obj->SetPosition(obj->GetPosition() - moveVector * 0.5f);
                    obj2->SetPosition(obj2->GetPosition() + moveVector * 0.5f);
                }
                physics1->Awake();
                physics2->Awake();
            }
            else if (physics1->GetBodyType() == BodyType3D::RIGID)
            {
                if (glm::length2(moveVector) > 0.0f)
                {
                    obj->SetPosition(obj->GetPosition() - moveVector);
                }
                physics1->Awake();
            }
            else if (physics2->GetBodyType() == BodyType3D::RIGID)
            {
                if (glm::length2(moveVector) > 0.0f)
                {
                    obj2->SetPosition(obj2->GetPosition() + moveVector);
                }
                physics2->Awake();
            }

            // Find valid contact points to apply rotational impulses
            const float contactTolerance = 0.001f;
            float maxDot1 = -FLT_MAX;
            for (const auto& p : rotatedPoly1)
            {
                maxDot1 = std::max(maxDot1, glm::dot(p, collisionNormal));
            }

            std::vector<glm::vec3> deepestPoints1;
            for (const auto& p : rotatedPoly1)
            {
                if (glm::dot(p, collisionNormal) > maxDot1 - contactTolerance)
                {
                    deepestPoints1.push_back(p);
                }
            }

            float maxDot2 = -FLT_MAX;
            for (const auto& p : rotatedPoly2)
            {
                maxDot2 = std::max(maxDot2, glm::dot(p, -collisionNormal));
            }

            std::vector<glm::vec3> deepestPoints2;
            for (const auto& p : rotatedPoly2)
            {
                if (glm::dot(p, -collisionNormal) > maxDot2 - contactTolerance)
                {
                    deepestPoints2.push_back(p);
                }
            }

            // Filter points to find those actually inside the other polygon's bounds
            glm::mat4 invTransform1 = glm::inverse(transform1);
            glm::mat4 invTransform2 = glm::inverse(transform2);
            glm::vec3 minLocal1(FLT_MAX), maxLocal1(-FLT_MAX);
            for (const auto& p : poly1)
            {
                minLocal1 = glm::min(minLocal1, p);
                maxLocal1 = glm::max(maxLocal1, p);
            }

            glm::vec3 minLocal2(FLT_MAX), maxLocal2(-FLT_MAX);
            for (const auto& p : poly2)
            {
                minLocal2 = glm::min(minLocal2, p);
                maxLocal2 = glm::max(maxLocal2, p);
            }

            const float eps = 0.01f;
            minLocal1 -= eps; maxLocal1 += eps;
            minLocal2 -= eps; maxLocal2 += eps;

            std::vector<glm::vec3> validContacts;
            for (const auto& p : deepestPoints1)
            {
                glm::vec3 localP = glm::vec3(invTransform2 * glm::vec4(p, 1.0f));
                if (localP.x >= minLocal2.x && localP.x <= maxLocal2.x && localP.y >= minLocal2.y && localP.y <= maxLocal2.y && localP.z >= minLocal2.z && localP.z <= maxLocal2.z)
                {
                    validContacts.push_back(p);
                }
            }
            for (const auto& p : deepestPoints2)
            {
                glm::vec3 localP = glm::vec3(invTransform1 * glm::vec4(p, 1.0f));
                if (localP.x >= minLocal1.x && localP.x <= maxLocal1.x && localP.y >= minLocal1.y && localP.y <= maxLocal1.y && localP.z >= minLocal1.z && localP.z <= maxLocal1.z)
                {
                    validContacts.push_back(p);
                }
            }

            // Apply linear and angular impulse at the contact centroid
            if (validContacts.empty())
            {
                glm::vec3 cp = deepestPoints1.empty() ? obj->GetPosition() : deepestPoints1[0];
                CalculateLinearVelocity(*physics1, *physics2, collisionNormal, &minDepth, cp);
            }
            else
            {
                glm::vec3 centroid(0.f);
                for (const auto& cp : validContacts)
                {
                    centroid += cp;
                }
                centroid /= static_cast<float>(validContacts.size());
                CalculateLinearVelocity(*physics1, *physics2, collisionNormal, &minDepth, centroid);
            }
        }
        return true;
    }
    return false;
}

bool Physics3D::CollisionSS(Object* obj, Object* obj2, CollisionMode mode)
{
    // Simple sphere-to-sphere distance check
    glm::vec3 center1 = obj->GetPosition();
    glm::vec3 center2 = obj2->GetPosition();
    float radius1 = obj->GetComponent<Physics3D>()->sphere.radius / 2.f;
    float radius2 = obj2->GetComponent<Physics3D>()->sphere.radius / 2.f;

    float distanceSquared = glm::length2(center2 - center1);
    float radiusSum = radius1 + radius2;

    if (distanceSquared <= radiusSum * radiusSum)
    {
        float distance = std::sqrt(distanceSquared);
        glm::vec3 normal = (center2 - center1) / distance;
        float depth = radiusSum - distance;

        if (mode == CollisionMode::COLLIDE && obj->GetComponent<Physics3D>()->GetIsGhostCollision() == false && obj2->GetComponent<Physics3D>()->GetIsGhostCollision() == false)
        {
            const float slop = 0.005f;
            float penetrationAmt = std::max(depth - slop, 0.0f);

            // Separate spheres based on their mass/rigid type
            if (obj->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::RIGID && obj2->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::RIGID)
            {
                if (penetrationAmt > 0.0f)
                {
                    obj->SetPosition(obj->GetPosition() - normal * penetrationAmt * 0.5f);
                    obj2->SetPosition(obj2->GetPosition() + normal * penetrationAmt * 0.5f);
                }
                obj->GetComponent<Physics3D>()->Awake();
                obj2->GetComponent<Physics3D>()->Awake();
            }
            else if (obj->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::RIGID && obj2->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::BLOCK)
            {
                if (penetrationAmt > 0.0f)
                {
                    obj->SetPosition(obj->GetPosition() - normal * penetrationAmt);
                }
                obj->GetComponent<Physics3D>()->Awake();
            }
            else if (obj->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::BLOCK && obj2->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::RIGID)
            {
                if (penetrationAmt > 0.0f)
                {
                    obj2->SetPosition(obj2->GetPosition() + normal * penetrationAmt);
                }
                obj2->GetComponent<Physics3D>()->Awake();
            }

            // Resolve collision impulse
            glm::vec3 cp = center1 - normal * radius1;
            CalculateLinearVelocity(*obj->GetComponent<Physics3D>(), *obj2->GetComponent<Physics3D>(), normal, &depth, cp);
        }
        return true;
    }
    return false;
}

bool Physics3D::CollisionPS(Object* poly, Object* sph, CollisionMode mode)
{
    // Polygon vs Sphere: Find the closest point on the polygon to the sphere center
    Physics3D* polyPhysics = poly->GetComponent<Physics3D>();
    Physics3D* sphPhysics = sph->GetComponent<Physics3D>();

    const glm::vec3 polyPos = poly->GetPosition();
    const glm::quat polyOrient = polyPhysics->GetEnableRotationalPhysics() ? polyPhysics->GetOrientation() : glm::quat(-glm::radians(poly->GetRotate3D()));

    const glm::vec3 sphereCenter = sph->GetPosition();
    const float sphereRadius = sphPhysics->sphere.radius / 2.f;

    if (polyPhysics->GetCollidePolyhedron().size() < 7)
    {
        return false;
    }

    glm::vec3 minExtent = polyPhysics->GetCollidePolyhedron()[0];
    glm::vec3 maxExtent = polyPhysics->GetCollidePolyhedron()[6];

    // Convert sphere center to the local coordinate space of the polygon
    glm::vec3 sphereCenterInLocal = glm::inverse(polyOrient) * (sphereCenter - polyPos);

    // Find the closest point in local space by clamping
    glm::vec3 closestPointInLocal;
    closestPointInLocal.x = std::max(minExtent.x, std::min(sphereCenterInLocal.x, maxExtent.x));
    closestPointInLocal.y = std::max(minExtent.y, std::min(sphereCenterInLocal.y, maxExtent.y));
    closestPointInLocal.z = std::max(minExtent.z, std::min(sphereCenterInLocal.z, maxExtent.z));

    float distSq = glm::length2(closestPointInLocal - sphereCenterInLocal);

    if (distSq <= (sphereRadius * sphereRadius))
    {
        if (mode == CollisionMode::COLLIDE && !polyPhysics->GetIsGhostCollision() && !sphPhysics->GetIsGhostCollision())
        {
            glm::vec3 normalInLocal = sphereCenterInLocal - closestPointInLocal;
            if (glm::length2(normalInLocal) < 0.0001f)
            {
                normalInLocal = -sphereCenterInLocal;
            }

            float dist = std::sqrt(distSq);
            float depth = sphereRadius - dist;
            const float slop = 0.005f;
            float penetrationAmt = std::max(depth - slop, 0.0f);

            // Transform local normal back to world space
            glm::vec3 normal = glm::normalize(polyOrient * normalInLocal);
            glm::vec3 moveVec = normal * penetrationAmt;

            // Apply positional correction
            if (polyPhysics->GetBodyType() == BodyType3D::RIGID && sphPhysics->GetBodyType() == BodyType3D::RIGID)
            {
                if (glm::length2(moveVec) > 0.0f)
                {
                    poly->SetPosition(poly->GetPosition() - moveVec * 0.5f);
                    sph->SetPosition(sph->GetPosition() + moveVec * 0.5f);
                }
                polyPhysics->Awake();
                sphPhysics->Awake();
            }
            else if (polyPhysics->GetBodyType() == BodyType3D::RIGID)
            {
                if (glm::length2(moveVec) > 0.0f)
                {
                    poly->SetPosition(poly->GetPosition() - moveVec);
                }
                polyPhysics->Awake();
            }
            else if (sphPhysics->GetBodyType() == BodyType3D::RIGID)
            {
                if (glm::length2(moveVec) > 0.0f)
                {
                    sph->SetPosition(sph->GetPosition() + moveVec);
                }
                sphPhysics->Awake();
            }

            // Apply impulse physics
            glm::vec3 cp = polyPos + (polyOrient * closestPointInLocal);
            CalculateLinearVelocity(*polyPhysics, *sphPhysics, normal, &depth, cp);
        }
        return true;
    }

    return false;
}

void Physics3D::AddCollidePolyhedron(glm::vec3 position)
{
    colliderType = ColliderType3D::BOX;
    collidePolyhedron.push_back(position);
}

void Physics3D::AddCollidePolyhedronAABB(glm::vec3 min, glm::vec3 max)
{
    // Define an axis-aligned bounding box from min/max corners
    colliderType = ColliderType3D::BOX;
    collidePolyhedron.clear();
    collidePolyhedron.push_back(glm::vec3(min.x, min.y, min.z));
    collidePolyhedron.push_back(glm::vec3(min.x, max.y, min.z));
    collidePolyhedron.push_back(glm::vec3(max.x, max.y, min.z));
    collidePolyhedron.push_back(glm::vec3(max.x, min.y, min.z));
    collidePolyhedron.push_back(glm::vec3(min.x, min.y, max.z));
    collidePolyhedron.push_back(glm::vec3(min.x, max.y, max.z));
    collidePolyhedron.push_back(glm::vec3(max.x, max.y, max.z));
    collidePolyhedron.push_back(glm::vec3(max.x, min.y, max.z));
}

void Physics3D::AddCollidePolyhedronAABB(glm::vec3 size)
{
    AddCollidePolyhedronAABB(-size / 2.f, size / 2.f);
}

void Physics3D::AddCollideSphere(float r)
{
    colliderType = ColliderType3D::SPHERE;
    collidePolyhedron.clear();
    sphere.radius = r;
}

glm::vec3 Physics3D::FindSATCenter(const std::vector<glm::vec3>& pts)
{
    // Calculate the arithmetic mean of a set of points
    glm::vec3 center(0.0f);
    if (pts.empty())
    {
        return center;
    }
    for (const auto& p : pts)
    {
        center += p;
    }
    return center / static_cast<float>(pts.size());
}

glm::vec3 Physics3D::RotatePoint(const glm::vec3& pt, const glm::vec3& pos, const glm::quat& rot)
{
    // Rotate a point around an origin and translate it
    return (rot * pt) + pos;
}

bool Physics3D::IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> pts1, const std::vector<glm::vec3> pts2, float* depth, float* mi1, float* ma1, float* mi2, float* ma2)
{
    // Project both polygons onto an axis to check for overlap
    float min1 = FLT_MAX;
    float max1 = -FLT_MAX;
    float min2 = FLT_MAX;
    float max2 = -FLT_MAX;

    for (const glm::vec3& v : pts1)
    {
        float p = glm::dot(axis, v);
        min1 = std::min(min1, p);
        max1 = std::max(max1, p);
    }
    for (const glm::vec3& v : pts2)
    {
        float p = glm::dot(axis, v);
        min2 = std::min(min2, p);
        max2 = std::max(max2, p);
    }

    // Determine the amount of overlap along the axis
    *depth = std::min(max2 - min1, max1 - min2);
    *mi1 = min1; *ma1 = max1; *mi2 = min2; *ma2 = max2;

    return !(max1 >= min2 && max2 >= min1);
}

bool Physics3D::IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> ptsPoly, const glm::vec3 ptSphere, const float r, float* depth, float* mi1, float* ma1, float* mi2, float* ma2)
{
    // Project polygon and sphere (represented as center +/- radius) onto an axis
    float min1 = FLT_MAX;
    float max1 = -FLT_MAX;

    for (const glm::vec3& p : ptsPoly)
    {
        float proj = glm::dot(p, axis);
        min1 = std::min(min1, proj);
        max1 = std::max(max1, proj);
    }

    float sphereProj = glm::dot(ptSphere, axis);
    float min2 = sphereProj - r;
    float max2 = sphereProj + r;

    *depth = std::min(max2 - min1, max1 - min2);
    *mi1 = min1; *ma1 = max1; *mi2 = min2; *ma2 = max2;

    return !(max1 >= min2 && max2 >= min1);
}

void Physics3D::CalculateLinearVelocity(Physics3D& body, Physics3D& body2, glm::vec3 normal, float* /*axisDepth*/, glm::vec3 contactPoint, float impulseScale)
{
    // Calculate lever arms from center of mass to contact point
    glm::vec3 ra = contactPoint - body.GetOwner()->GetPosition();
    glm::vec3 rb = contactPoint - body2.GetOwner()->GetPosition();

    // Calculate contact velocities including rotational components
    glm::vec3 vaContact = body.GetVelocity() + glm::cross(body.GetAngularVelocity(), ra);
    glm::vec3 vbContact = body2.GetVelocity() + glm::cross(body2.GetAngularVelocity(), rb);

    // Get the relative velocity along the normal
    glm::vec3 relativeVelocity = vbContact - vaContact;
    float velAlongNormal = glm::dot(relativeVelocity, normal);

    // Do not resolve if objects are already separating
    if (velAlongNormal > 0.0f)
    {
        return;
    }

    float res = std::min(body.GetRestitution(), body2.GetRestitution());

    // Compute rotational effect on the impulse denominator
    glm::vec3 raCrossN = glm::cross(ra, normal);
    glm::vec3 rbCrossN = glm::cross(rb, normal);

    float invMassSum = (body.GetBodyType() == BodyType3D::RIGID ? (1.f / body.mass) : 0.0f) + (body2.GetBodyType() == BodyType3D::RIGID ? (1.f / body2.mass) : 0.0f);

    float denominator = invMassSum;
    if (body.GetBodyType() == BodyType3D::RIGID && body.GetEnableRotationalPhysics())
    {
        glm::vec3 angEffectA = glm::cross(raCrossN * body.GetInverseInertia(), ra);
        denominator += glm::dot(angEffectA, normal);
    }
    if (body2.GetBodyType() == BodyType3D::RIGID && body2.GetEnableRotationalPhysics())
    {
        glm::vec3 angEffectB = glm::cross(rbCrossN * body2.GetInverseInertia(), rb);
        denominator += glm::dot(angEffectB, normal);
    }

    if (denominator <= 0.0f)
    {
        return;
    }

    // Calculate the impulse magnitude j
    float j = -(1.f + res) * velAlongNormal / denominator;
    j *= impulseScale;
    glm::vec3 impulse = normal * j;

    // Apply the computed impulse to both bodies
    if (body.GetBodyType() == BodyType3D::RIGID)
    {
        body.SetVelocity(body.GetVelocity() - impulse * (1.f / body.mass));
        if (body.GetEnableRotationalPhysics())
        {
            body.SetAngularVelocity(body.GetAngularVelocity() - glm::cross(ra, impulse) * body.GetInverseInertia());
        }
    }
    if (body2.GetBodyType() == BodyType3D::RIGID)
    {
        body2.SetVelocity(body2.GetVelocity() + impulse * (1.f / body2.mass));
        if (body2.GetEnableRotationalPhysics())
        {
            body2.SetAngularVelocity(body2.GetAngularVelocity() + glm::cross(rb, impulse) * body2.GetInverseInertia());
        }
    }
}

glm::vec3 Physics3D::FindClosestPointOnSegment(const glm::vec3& cpSphere, std::vector<glm::vec3>& verts)
{
    // Find the point on the polygon boundary closest to the sphere center
    if (verts.empty())
    {
        return glm::vec3(0.0f);
    }
    glm::vec3 resultPoint = verts[0];
    float minDistanceSquared = FLT_MAX;

    for (size_t i = 0; i < verts.size(); i++)
    {
        glm::vec3 va = verts[i];
        glm::vec3 vb = verts[(i + 1) % verts.size()];
        glm::vec3 edge = vb - va;
        glm::vec3 toSphere = cpSphere - va;
        float lenSq = glm::length2(edge);
        float t = 0.0f;
        if (lenSq > 0.0f)
        {
            // Project to find the normalized parameter t along the edge
            t = std::max(0.0f, std::min(1.0f, glm::dot(toSphere, edge) / lenSq));
        }
        glm::vec3 closestPoint = va + edge * t;
        float dSq = glm::length2(closestPoint - cpSphere);
        if (dSq < minDistanceSquared)
        {
            minDistanceSquared = dSq;
            resultPoint = closestPoint;
        }
    }
    return resultPoint;
}

void Physics3D::ProjectPolygon(const std::vector<glm::vec3>& verts, const glm::vec3& axis, float& min, float& max)
{
    // Utility to project a list of vertices onto a single axis
    if (verts.empty())
    {
        return;
    }
    min = glm::dot(verts[0], axis);
    max = min;
    for (size_t i = 1; i < verts.size(); ++i)
    {
        float p = glm::dot(verts[i], axis);
        if (p < min)
        {
            min = p;
        }
        else if (p > max)
        {
            max = p;
        }
    }
}

bool Physics3D::SweptSATOBB(Physics3D* body1, Physics3D* body2, float dt, CollisionResult& res)
{
    // High-performance swept collision detection using SAT
    Object* o1 = body1->GetOwner();
    Object* o2 = body2->GetOwner();
    const auto& poly1 = body1->GetCollidePolyhedron();
    const auto& poly2 = body2->GetCollidePolyhedron();
    if (poly1.empty() || poly2.empty())
    {
        return false;
    }

    // Determine world-space vertices
    std::vector<glm::vec3> rotPoly1, rotPoly2;
    glm::quat orient1 = body1->GetEnableRotationalPhysics() ? body1->GetOrientation() : glm::quat(glm::radians(o1->GetRotate3D()));
    glm::quat orient2 = body2->GetEnableRotationalPhysics() ? body2->GetOrientation() : glm::quat(glm::radians(o2->GetRotate3D()));
    glm::mat4 rMat1 = glm::mat4_cast(orient1);
    glm::mat4 rMat2 = glm::mat4_cast(orient2);
    glm::mat4 t1 = glm::translate(glm::mat4(1.0f), o1->GetPosition()) * rMat1;
    glm::mat4 t2 = glm::translate(glm::mat4(1.0f), o2->GetPosition()) * rMat2;

    for (const auto& p : poly1)
    {
        rotPoly1.emplace_back(glm::vec3(t1 * glm::vec4(p, 1.0f)));
    }
    for (const auto& p : poly2)
    {
        rotPoly2.emplace_back(glm::vec3(t2 * glm::vec4(p, 1.0f)));
    }

    // Check for static intersection first
    glm::vec3 iNorm;
    float iDepth;
    if (StaticSATIntersection(body1, body2, rotPoly1, rotPoly2, rMat1, rMat2, iNorm, iDepth))
    {
        res.hasCollided = true;
        res.timeOfImpact = 0.0f;
        res.otherObject = o2;
        res.collisionNormal = iNorm;
        return true;
    }

    // Calculate relative movement per frame
    glm::vec3 relVel = (body2->GetVelocity() - body1->GetVelocity()) * dt;
    std::vector<glm::vec3> axes;
    glm::vec3 a1[3] = { glm::normalize(glm::vec3(rMat1[0])), glm::normalize(glm::vec3(rMat1[1])), glm::normalize(glm::vec3(rMat1[2])) };
    glm::vec3 a2[3] = { glm::normalize(glm::vec3(rMat2[0])), glm::normalize(glm::vec3(rMat2[1])), glm::normalize(glm::vec3(rMat2[2])) };
    for (int i = 0; i < 3; ++i)
    {
        axes.push_back(a1[i]);
        axes.push_back(a2[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            glm::vec3 c = glm::cross(a1[i], a2[j]);
            if (glm::length2(c) > 0.0001f)
            {
                axes.push_back(glm::normalize(c));
            }
        }
    }

    // Iterate through axes to find the first and last time of contact
    float tFirst = 0.0f, tLast = 1.0f;
    glm::vec3 bestNorm(0.0f);
    for (const auto& axis : axes)
    {
        float min1, max1, min2, max2;
        ProjectPolygon(rotPoly1, axis, min1, max1);
        ProjectPolygon(rotPoly2, axis, min2, max2);
        float v = glm::dot(relVel, axis);
        if (max1 < min2)
        {
            if (v <= 0.0f)
            {
                return false;
            }
            float t = (min2 - max1) / v;
            if (t > tFirst)
            {
                tFirst = t; bestNorm = -axis;
            }
        }
        else if (max2 < min1)
        {
            if (v >= 0.0f)
            {
                return false;
            }
            float t = (max2 - min1) / v;
            if (t > tFirst)
            {
                tFirst = t; bestNorm = axis;
            }
        }

        // Update exit time
        if (v > 0.0f)
        {
            tLast = std::min(tLast, (max2 - min1) / v);
        }
        else if (v < 0.0f)
        {
            tLast = std::min(tLast, (min2 - max1) / v);
        }

        if (tFirst > tLast)
        {
            return false;
        }
    }

    if (tFirst >= 0.0f && tFirst <= 1.0f)
    {
        res.hasCollided = true;
        res.timeOfImpact = tFirst;
        res.otherObject = o2;
        res.collisionNormal = glm::normalize(bestNorm);
        return true;
    }
    return false;
}

bool Physics3D::StaticSATIntersection(Physics3D* /*b1*/, Physics3D* /*b2*/, const std::vector<glm::vec3>& rp1, const std::vector<glm::vec3>& rp2, const glm::mat4& rm1, const glm::mat4& rm2, glm::vec3& oNorm, float& oDepth)
{
    // Basic SAT test for static intersection (non-swept)
    std::vector<glm::vec3> axes;
    glm::vec3 a1[3] = { glm::normalize(glm::vec3(rm1[0])), glm::normalize(glm::vec3(rm1[1])), glm::normalize(glm::vec3(rm1[2])) };
    glm::vec3 a2[3] = { glm::normalize(glm::vec3(rm2[0])), glm::normalize(glm::vec3(rm2[1])), glm::normalize(glm::vec3(rm2[2])) };

    for (int i = 0; i < 3; ++i)
    {
        axes.push_back(a1[i]);
        axes.push_back(a2[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            glm::vec3 c = glm::cross(a1[i], a2[j]);
            if (glm::length2(c) > 0.0001f)
            {
                axes.push_back(glm::normalize(c));
            }
        }
    }

    float mDepth = FLT_MAX;
    glm::vec3 cNorm(0.0f);
    for (const auto& a : axes)
    {
        float min1, max1, min2, max2;
        ProjectPolygon(rp1, a, min1, max1);
        ProjectPolygon(rp2, a, min2, max2);

        if (max1 < min2 || max2 < min1)
        {
            return false;
        }

        float d = std::min(max2 - min1, max1 - min2);
        if (d < mDepth)
        {
            mDepth = d;
            cNorm = (max1 - min2) < (max2 - min1) ? a : -a;
        }
    }
    oNorm = glm::normalize(cNorm);
    oDepth = mDepth;
    return true;
}

bool Physics3D::SweptSpheres(Physics3D* b1, Physics3D* b2, float dt, CollisionResult& res)
{
    // Continuous collision detection between two moving spheres
    Object* o1 = b1->GetOwner(), * o2 = b2->GetOwner();
    glm::vec3 p1 = o1->GetPosition(), p2 = o2->GetPosition();
    float r1 = b1->sphere.radius / 2.0f, r2 = b2->sphere.radius / 2.0f, rSum = r1 + r2;
    glm::vec3 vRel = (b2->GetVelocity() - b1->GetVelocity()) * dt;
    glm::vec3 diff = p2 - p1;

    // Solve quadratic equation for moving distance: |(P+tV) - (P')|^2 = (r+r')^2
    float a = glm::dot(vRel, vRel);
    float b = 2.0f * glm::dot(diff, vRel);
    float c = glm::dot(diff, diff) - rSum * rSum;

    if (c < 0.0f)
    {
        res.hasCollided = true; res.timeOfImpact = 0.0f; res.otherObject = o2;
        res.collisionNormal = glm::normalize(p1 - p2); return true;
    }
    if (a < 0.00001f)
    {
        return false;
    }

    float det = b * b - 4 * a * c;
    if (det < 0)
    {
        return false;
    }

    float t = (-b - sqrt(det)) / (2.0f * a);
    if (t >= 0.0f && t <= 1.0f)
    {
        res.hasCollided = true; res.timeOfImpact = t; res.otherObject = o2;
        res.collisionNormal = glm::normalize((p1 + b1->GetVelocity() * dt * t) - (p2 + b2->GetVelocity() * dt * t));
        return true;
    }
    return false;
}

bool Physics3D::SweptSphereVsOBB(Physics3D* b2, float dt, CollisionResult& res)
{
    // Test moving sphere against a static/rotating Oriented Bounding Box
    Object* o1 = GetOwner(), * o2 = b2->GetOwner();
    glm::quat orient2 = b2->GetEnableRotationalPhysics() ? b2->GetOrientation() : glm::quat(glm::radians(o2->GetRotate3D()));
    glm::mat4 rm2 = glm::mat4_cast(orient2);
    glm::mat4 im2 = glm::inverse(glm::translate(glm::mat4(1.0f), o2->GetPosition()) * rm2);

    // Convert relative motion into OBB local space
    glm::vec3 p1l = glm::vec3(im2 * glm::vec4(o1->GetPosition(), 1.0f));
    glm::vec3 v1l = glm::vec3(im2 * glm::vec4(GetVelocity() * dt, 0.0f));
    glm::vec3 h2 = b2->GetCollidePolyhedron().size() > 6 ? b2->GetCollidePolyhedron()[6] : glm::vec3(1.0f);

    float r1 = sphere.radius / 2.0f, tFirst = 0.0f, tLast = 1.0f;
    glm::vec3 nL(0.0f);

    // Slab method for AABB/OBB collision in local space
    for (int i = 0; i < 3; ++i)
    {
        float sMin = -h2[i] - r1, sMax = h2[i] + r1;
        if (std::abs(v1l[i]) < 0.00001f)
        {
            if (p1l[i] < sMin || p1l[i] > sMax)
            {
                return false;
            }
        }
        else
        {
            float tE = (sMin - p1l[i]) / v1l[i];
            float tL = (sMax - p1l[i]) / v1l[i];
            if (tE > tL)
            {
                std::swap(tE, tL);
            }
            if (tE > tFirst)
            {
                tFirst = tE; nL = glm::vec3(0.0f); nL[i] = (v1l[i] > 0) ? -1.0f : 1.0f;
            }
            tLast = std::min(tLast, tL);
            if (tFirst > tLast)
            {
                return false;
            }
        }
    }

    if (tFirst == 0.0f)
    {
        glm::vec3 cp = glm::clamp(p1l, -h2, h2);
        glm::vec3 d = p1l - cp;
        nL = glm::length2(d) < 0.0001f ? -glm::normalize(p1l) : glm::normalize(d);
    }
    if (tFirst >= 0.0f && tFirst <= 1.0f)
    {
        res.hasCollided = true; res.timeOfImpact = tFirst; res.otherObject = o2;
        res.collisionNormal = glm::normalize(orient2 * nL);
        return true;
    }
    return false;
}

CollisionResult Physics3D::FindClosestCollision(float dt)
{
    // Search all scene objects to find the earliest collision in this frame
    CollisionResult res;
    res.timeOfImpact = 1.1f;
    Object* s = GetOwner();

    for (auto& p : Engine::GetObjectManager().GetObjectMap())
    {
        Object* o = p.second.get();
        if (s == o || !o->HasComponent<Physics3D>())
        {
            continue;
        }

        Physics3D* b = o->GetComponent<Physics3D>();
        CollisionResult cur;
        ColliderType3D t1 = GetColliderType();
        ColliderType3D t2 = b->GetColliderType();
        bool hit = false;

        if (t1 == ColliderType3D::SPHERE && t2 == ColliderType3D::SPHERE)
        {
            hit = SweptSpheres(this, b, dt, cur);
        }
        else if (t1 == ColliderType3D::BOX && t2 == ColliderType3D::BOX)
        {
            hit = SweptSATOBB(this, b, dt, cur);
        }
        else if (t1 == ColliderType3D::SPHERE && t2 == ColliderType3D::BOX)
        {
            hit = SweptSphereVsOBB(b, dt, cur);
        }
        else if (t1 == ColliderType3D::BOX && t2 == ColliderType3D::SPHERE)
        {
            if (b->SweptSphereVsOBB(this, dt, cur))
            {
                cur.collisionNormal *= -1.0f; hit = true;
            }
        }

        // Track only the earliest impact
        if (hit && cur.timeOfImpact < res.timeOfImpact)
        {
            res = cur;
        }
    }
    return res;
}