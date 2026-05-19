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

    // Apply Newton's second law of motion where Force equals mass times acceleration to calculate the current acceleration vector
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

    // Reset the accumulated force back to zero after integration so forces do not carry over to the next frame
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
        // Calculate angular acceleration from torque using the inverse moment of inertia tensor equivalent
        glm::vec3 angularAcceleration = torque * inverseInertia;
        angularVelocity += angularAcceleration * dt;

        // Apply rotational damping to simulate air resistance and reduce endless spinning
        angularVelocity *= (1.f - angularDamping * dt);

        // Clamp extremely low angular velocities to zero
        if (glm::length2(angularVelocity) < 1e-6f)
        {
            angularVelocity = glm::vec3(0.0f);
        }

        torque = glm::vec3(0.0f);

        // Perform quaternion integration to update orientation based on the angular velocity vector scaled by half the delta time
        glm::quat spin = 0.5f * glm::quat(0.f, angularVelocity.x, angularVelocity.y, angularVelocity.z) * orientation;
        orientation.w += spin.w * dt;
        orientation.x += spin.x * dt;
        orientation.y += spin.y * dt;
        orientation.z += spin.z * dt;
        orientation = glm::normalize(orientation);

        // Map the quaternion orientation back to Euler angles to synchronize with the visual object transform
        glm::vec3 eulerArgs = glm::degrees(glm::eulerAngles(orientation));
        GetOwner()->SetRotate(-eulerArgs);
    }

    // Set thresholds for kinetic energy below which the object is considered stationary and should go to sleep to save processing power
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

            // Calculate the exact point of contact by retracting slightly along the normal vector and resolve the linear velocity impulse
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
        // Simple discrete Euler integration movement without continuous collision checks
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
        const auto& poly1 = physics1->GetCollidePolyhedron();
        const auto& poly2 = physics2->GetCollidePolyhedron();

        if (poly1.empty() || poly2.empty())
        {
            return false;
        }

        // thread_local prevents data corruption when multiple worker threads access these buffers simultaneously
        static thread_local std::vector<glm::vec3> rotatedPoly1;
        static thread_local std::vector<glm::vec3> rotatedPoly2;

        // reset buffers without deallocating memory for performance
        rotatedPoly1.clear();
        rotatedPoly2.clear();

        // fetch orientations based on whether rotational physics is enabled
        glm::quat orient1 = physics1->GetEnableRotationalPhysics() ? physics1->GetOrientation() : glm::quat(-glm::radians(obj->GetRotate3D()));
        glm::quat orient2 = physics2->GetEnableRotationalPhysics() ? physics2->GetOrientation() : glm::quat(-glm::radians(obj2->GetRotate3D()));

        glm::mat4 rotationMatrix1 = glm::mat4_cast(orient1);
        glm::mat4 rotationMatrix2 = glm::mat4_cast(orient2);

        // calculate world space transformation matrices
        glm::mat4 transform1 = glm::translate(glm::mat4(1.0f), obj->GetPosition()) * rotationMatrix1;
        glm::mat4 transform2 = glm::translate(glm::mat4(1.0f), obj2->GetPosition()) * rotationMatrix2;

        // transform all local vertices to world space coordinates
        for (const auto& point : poly1)
        {
            rotatedPoly1.emplace_back(glm::vec3(transform1 * glm::vec4(point, 1.0f)));
        }

        for (const auto& point : poly2)
        {
            rotatedPoly2.emplace_back(glm::vec3(transform2 * glm::vec4(point, 1.0f)));
        }

        GjkShape shapeA;
        shapeA.type = ColliderType3D::BOX;
        shapeA.vertices = &rotatedPoly1;

        GjkShape shapeB;
        shapeB.type = ColliderType3D::BOX;
        shapeB.vertices = &rotatedPoly2;
        // Execute the Gilbert-Johnson-Keerthi distance algorithm to detect if the two convex hulls intersect by building a simplex inside their Minkowski Difference
        std::vector<SupportPoint> finalSimplex;
        bool hasCollision = CheckCollisionGJK(shapeA, shapeB, finalSimplex);

        if (hasCollision == false)
        {
            return false;
        }

        // Expand the polytope using the Expanding Polytope Algorithm to find the exact collision normal, penetration depth, and true contact point on the surface
        glm::vec3 contactNormal(0.0f);
        float contactDepth = 0.0f;
        glm::vec3 actualContactPoint(0.0f);
        CalculatePenetrationEPA(shapeA, shapeB, finalSimplex, contactNormal, contactDepth, actualContactPoint);
        
        if (mode == CollisionMode::COLLIDE && !physics1->GetIsGhostCollision() && !physics2->GetIsGhostCollision())
        {
            const float skinSlop = 0.005f;
            const float resolutionRate = 0.2f;
            float actualPenetration = std::max(contactDepth - skinSlop, 0.0f);
            glm::vec3 moveDelta = contactNormal * (actualPenetration * resolutionRate);

            // calculate relative velocity to ignore micro resting collisions
            glm::vec3 relativeVelocity = physics1->GetVelocity() - physics2->GetVelocity();

            // only wake objects if the impact has noticeable speed or deep penetration
            bool isSignificantImpact = (actualPenetration > 0.001f) || (glm::length2(relativeVelocity) > 0.01f);

            if (physics1->GetBodyType() == BodyType3D::RIGID && physics2->GetBodyType() == BodyType3D::RIGID)
            {
                if (glm::length2(moveDelta) > 0.0f)
                {
                    obj->SetPosition(obj->GetPosition() - moveDelta * 0.5f);
                    obj2->SetPosition(obj2->GetPosition() + moveDelta * 0.5f);
                }

                if (isSignificantImpact)
                {
                    physics1->Awake();
                    physics2->Awake();
                }
            }
            else if (physics1->GetBodyType() == BodyType3D::RIGID)
            {
                if (glm::length2(moveDelta) > 0.0f)
                {
                    obj->SetPosition(obj->GetPosition() - moveDelta);
                }

                if (isSignificantImpact)
                {
                    physics1->Awake();
                }
            }
            else if (physics2->GetBodyType() == BodyType3D::RIGID)
            {
                if (glm::length2(moveDelta) > 0.0f)
                {
                    obj2->SetPosition(obj2->GetPosition() + moveDelta);
                }

                if (isSignificantImpact)
                {
                    physics2->Awake();
                }
            }
            CalculateLinearVelocity(*physics1, *physics2, contactNormal, &contactDepth, actualContactPoint);
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
    auto* physicsPoly = poly->GetComponent<Physics3D>();
    auto* physicsSph = sph->GetComponent<Physics3D>();

    if (physicsPoly->GetCollidePolyhedron().empty())
    {
        return false;
    }

    const auto& polyVertices = physicsPoly->GetCollidePolyhedron();

    static thread_local std::vector<glm::vec3> rotatedPoly;
    rotatedPoly.clear();

    glm::quat polyOrient = physicsPoly->GetEnableRotationalPhysics() ? physicsPoly->GetOrientation() : glm::quat(-glm::radians(poly->GetRotate3D()));
    glm::mat4 polyMatrix = glm::translate(glm::mat4(1.0f), poly->GetPosition()) * glm::mat4_cast(polyOrient);

    for (const auto& point : polyVertices)
    {
        rotatedPoly.emplace_back(glm::vec3(polyMatrix * glm::vec4(point, 1.0f)));
    }

    // setup shape proxy for polygon
    GjkShape shapePoly;
    shapePoly.type = ColliderType3D::BOX;
    shapePoly.vertices = &rotatedPoly;

    // setup shape proxy for sphere
    GjkShape shapeSphere;
    shapeSphere.type = ColliderType3D::SPHERE;
    shapeSphere.center = sph->GetPosition();
    shapeSphere.radius = physicsSph->GetSphereRadius() / 2.0f;

    std::vector<SupportPoint> finalSimplex;
    bool hasCollision = CheckCollisionGJK(shapePoly, shapeSphere, finalSimplex);

    if (hasCollision == false)
    {
        return false;
    }

    glm::vec3 contactNormal(0.0f);
    float contactDepth = 0.0f;
    glm::vec3 actualContactPoint(0.0f);

    CalculatePenetrationEPA(shapePoly, shapeSphere, finalSimplex, contactNormal, contactDepth, actualContactPoint);

    if (mode == CollisionMode::COLLIDE && !physicsPoly->GetIsGhostCollision() && !physicsSph->GetIsGhostCollision())
    {
        const float skinSlop = 0.005f;
        const float resolutionRate = 0.2f;
        float actualPenetration = std::max(contactDepth - skinSlop, 0.0f);
        glm::vec3 moveDelta = contactNormal * (actualPenetration * resolutionRate);

        // calculate relative velocity to ignore micro resting collisions
        glm::vec3 relativeVelocity = physicsPoly->GetVelocity() - physicsSph->GetVelocity();
        bool isSignificantImpact = (actualPenetration > 0.001f) || (glm::length2(relativeVelocity) > 0.01f);
        
        if (physicsPoly->GetBodyType() == BodyType3D::RIGID && physicsSph->GetBodyType() == BodyType3D::RIGID)
        {
            if (glm::length2(moveDelta) > 0.0f)
            {
                poly->SetPosition(poly->GetPosition() - moveDelta * 0.5f);
                sph->SetPosition(sph->GetPosition() + moveDelta * 0.5f);
            }

            if (isSignificantImpact)
            {
                physicsPoly->Awake();
                physicsSph->Awake();
            }
        }
        else if (physicsPoly->GetBodyType() == BodyType3D::RIGID)
        {
            if (glm::length2(moveDelta) > 0.0f)
            {
                poly->SetPosition(poly->GetPosition() - moveDelta);
            }

            if (isSignificantImpact)
            {
                physicsPoly->Awake();
            }
        }
        else if (physicsSph->GetBodyType() == BodyType3D::RIGID)
        {
            if (glm::length2(moveDelta) > 0.0f)
            {
                sph->SetPosition(sph->GetPosition() + moveDelta);
            }

            if (isSignificantImpact)
            {
                physicsSph->Awake();
            }
        }
        CalculateLinearVelocity(*physicsPoly, *physicsSph, contactNormal, &contactDepth, actualContactPoint);
    }
    return true;
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

glm::vec3 Physics3D::ComputePolygonCenter(const std::vector<glm::vec3>& pts)
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

void Physics3D::CalculateLinearVelocity(Physics3D& body, Physics3D& body2, glm::vec3 normal, float* /*axisDepth*/, glm::vec3 contactPoint, float impulseScale)
{
    // Calculate the lever arm vectors from the center of mass of each body to the point of contact.
    // These vectors are used to compute the torque generated by the collision impulse.
    glm::vec3 ra = contactPoint - body.GetOwner()->GetPosition();
    glm::vec3 rb = contactPoint - body2.GetOwner()->GetPosition();

    // Calculate the total velocity at the contact point for both bodies.
    // This includes both the linear velocity and the linear velocity induced by the angular rotation at the contact point.
    glm::vec3 vaContact = body.GetVelocity() + glm::cross(body.GetAngularVelocity(), ra);
    glm::vec3 vbContact = body2.GetVelocity() + glm::cross(body2.GetAngularVelocity(), rb);

    // Determine the relative velocity between the two bodies along the collision normal.
    // This value indicates how fast the bodies are moving towards or away from each other.
    glm::vec3 relativeVelocity = vbContact - vaContact;
    float velAlongNormal = glm::dot(relativeVelocity, normal);

    // Do not resolve the collision if the relative velocity is positive, which means the objects are already separating.
    if (velAlongNormal > 0.0f)
    {
        return;
    }

    float res = std::min(body.GetRestitution(), body2.GetRestitution());

    // Apply a velocity threshold to prevent infinite micro bouncing caused by constant gravitational acceleration.
    if (std::abs(velAlongNormal) < 0.2f)
    {
        res = 0.0f;
    }

    // Compute the rotational effect on the impulse denominator.
    // This represents how much the bodies will rotate in response to an impulse applied at the contact point, based on their inertia tensors.
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

    // Calculate the scalar magnitude of the impulse j.
    // The formula incorporates the coefficient of restitution to simulate bounciness and divides by the combined mass and inertia denominator.
    float j = -(1.f + res) * velAlongNormal / denominator;
    j *= impulseScale;
    glm::vec3 impulse = normal * j;

    // Apply the computed impulse vector to the linear and angular velocities of both rigid bodies.
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

bool Physics3D::SweptSpheres(Physics3D* b1, Physics3D* b2, float dt, CollisionResult& res)
{
    // Continuous collision detection between two moving spheres by modeling their movement as a quadratic equation over time.
    Object* o1 = b1->GetOwner(), * o2 = b2->GetOwner();
    glm::vec3 p1 = o1->GetPosition(), p2 = o2->GetPosition();
    float r1 = b1->sphere.radius / 2.0f, r2 = b2->sphere.radius / 2.0f, rSum = r1 + r2;
    glm::vec3 vRel = (b2->GetVelocity() - b1->GetVelocity()) * dt;
    glm::vec3 diff = p2 - p1;

    // Solve the quadratic equation for the time of intersection.
    // The equation models the distance between the two spheres over time: |(Position1 + Time * Velocity1) - (Position2 + Time * Velocity2)|^2 = (Radius1 + Radius2)^2.
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



CollisionResult Physics3D::FindClosestCollision(float dt)
{
    // Search all scene objects to find the earliest continuous collision time of impact in this frame.
    CollisionResult result;
    result.timeOfImpact = 1.1f;
    Object* ownerObject = GetOwner();

    // Query the Bounding Volume Hierarchy tree to retrieve only the objects that are near the swept path of this body.
    // This is a massive performance optimization over checking every object in the scene.
    std::vector<Physics3D*> potentialColliders = Engine::GetPhysicsManager().GetPotentialColliders(this, dt);

    // Iterate through the filtered list of potential colliders obtained from the spatial partition.
    for (Physics3D* otherBody : potentialColliders)
    {
        Object* otherObject = otherBody->GetOwner();

        CollisionResult currentResult;
        ColliderType3D myType = GetColliderType();
        ColliderType3D otherType = otherBody->GetColliderType();
        bool isHit = false;

        // Dispatch continuous collision detection routines based on the shape types of the two bodies to calculate exact time of impact.
        if (myType == ColliderType3D::SPHERE && otherType == ColliderType3D::SPHERE)
        {
            isHit = SweptSpheres(this, otherBody, dt, currentResult);
        }
        else
        {
            // Box-Box, Sphere-Box, Box-Sphere
            isHit = SweptGJK(this, otherBody, dt, currentResult);

            // invert normal if the calling body is sphere hitting a box to maintain standard reaction
            if (isHit && myType == ColliderType3D::SPHERE && otherType == ColliderType3D::BOX)
            {
                currentResult.collisionNormal *= -1.0f;
            }
        }

        // Track only the earliest impact time
        if (isHit)
        {
            if (currentResult.timeOfImpact < result.timeOfImpact)
            {
                result = currentResult;
            }
        }
    }

    return result;
}

glm::vec3 Physics3D::GetShapeSupportPoint(const GjkShape& shape, glm::vec3 searchDirection)
{
    // Calculate the support point of a shape in a given direction.
    // The support point is the vertex of the convex shape that is furthest in the specified search direction.
    if (shape.type == ColliderType3D::BOX)
    {
        float maxDistance = -FLT_MAX;
        glm::vec3 farthestPoint(0.0f);

        // Iterate through all vertices of the polyhedron to find the one with the maximum projection along the search direction.
        for (const auto& vertex : *(shape.vertices))
        {
            // Apply dynamic offset for continuous collision detection sweeping.
            glm::vec3 shiftedVertex = vertex + shape.offset;
            float currentDistance = glm::dot(shiftedVertex, searchDirection);

            if (currentDistance > maxDistance)
            {
                maxDistance = currentDistance;
                farthestPoint = shiftedVertex;
            }
        }
        return farthestPoint;
    }
    else if (shape.type == ColliderType3D::SPHERE)
    {
        // For a sphere, the support point is simply the center plus the radius in the normalized search direction.
        glm::vec3 normalizedDirection = glm::normalize(searchDirection);
        // Apply dynamic offset to the sphere center for sweep calculations.
        return (shape.center + shape.offset) + normalizedDirection * shape.radius;
    }

    return glm::vec3(0.0f);
}

SupportPoint Physics3D::GetSupport(const GjkShape& shapeA, const GjkShape& shapeB, glm::vec3 searchDirection)
{
    // The support function for the Minkowski Difference of two convex shapes A and B.
    // The Minkowski Difference is defined as the set of all points (a - b) where a is in A and b is in B.
    // A support point of the Minkowski Difference in direction D is calculated as Support(A, D) - Support(B, -D).
    SupportPoint newSupport;

    newSupport.pointOnA = GetShapeSupportPoint(shapeA, searchDirection);
    newSupport.pointOnB = GetShapeSupportPoint(shapeB, -searchDirection);
    newSupport.minkowskiDifference = newSupport.pointOnA - newSupport.pointOnB;

    return newSupport;
}

// update checkCollisionGJK signature and center calculations
bool Physics3D::CheckCollisionGJK(const GjkShape& shapeA, const GjkShape& shapeB, std::vector<SupportPoint>& outSimplex)
{
    // The Gilbert-Johnson-Keerthi (GJK) algorithm determines if two convex shapes intersect by checking if their Minkowski Difference contains the origin.
    // We iteratively build a simplex (a point, line, triangle, or tetrahedron) within the Minkowski Difference to enclose the origin.
    
    // Apply sweep offsets to shape centers to find an initial search direction towards each other.
    glm::vec3 centerA = (shapeA.type == ColliderType3D::BOX) ? ComputePolygonCenter(*(shapeA.vertices)) + shapeA.offset : shapeA.center + shapeA.offset;
    glm::vec3 centerB = (shapeB.type == ColliderType3D::BOX) ? ComputePolygonCenter(*(shapeB.vertices)) + shapeB.offset : shapeB.center + shapeB.offset;

    glm::vec3 searchDirection = centerA - centerB;

    // Use a default direction if the centers are nearly identical to avoid mathematical errors.
    if (glm::length2(searchDirection) < 0.0001f)
    {
        searchDirection = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    // Initialize the simplex with the first support point in the initial search direction.
    SupportPoint firstPoint = GetSupport(shapeA, shapeB, searchDirection);
    outSimplex.push_back(firstPoint);

    // The next search direction is always towards the origin from the last point added.
    searchDirection = -firstPoint.minkowskiDifference;

    // Limit iterations to prevent infinite loops in cases of numerical instability.
    for (int i = 0; i < 64; ++i)
    {
        SupportPoint nextPoint = GetSupport(shapeA, shapeB, searchDirection);

        // If the new support point does not pass the origin in the search direction, the Minkowski Difference cannot contain the origin.
        if (glm::dot(nextPoint.minkowskiDifference, searchDirection) < 0.0f)
        {
            return false;
        }

        outSimplex.push_back(nextPoint);

        // Update the simplex and search direction. If HandleSimplex returns true, the simplex now encloses the origin.
        if (HandleSimplex(outSimplex, searchDirection))
        {
            return true;
        }
    }

    return false;
}

bool Physics3D::HandleSimplex(std::vector<SupportPoint>& currentSimplex, glm::vec3& currentDirection)
{
    // Logic for a line segment simplex (two points).
    if (currentSimplex.size() == 2)
    {
        // Calculate the vector of the line and the vector from the latest point to the origin.
        glm::vec3 lineAB = currentSimplex[0].minkowskiDifference - currentSimplex[1].minkowskiDifference;
        glm::vec3 toOrigin = -currentSimplex[1].minkowskiDifference;

        // Use triple product to find a direction perpendicular to the line and pointing towards the origin.
        currentDirection = glm::cross(glm::cross(lineAB, toOrigin), lineAB);

        // If the origin lies exactly on the line, choose an arbitrary perpendicular direction.
        if (glm::length2(currentDirection) < 0.0001f)
        {
            currentDirection = glm::cross(lineAB, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        return false;
    }

    // Logic for a triangle simplex (three points).
    if (currentSimplex.size() == 3)
    {
        // Calculate edges from the latest point C added to the simplex.
        glm::vec3 edgeAC = currentSimplex[0].minkowskiDifference - currentSimplex[2].minkowskiDifference;
        glm::vec3 edgeBC = currentSimplex[1].minkowskiDifference - currentSimplex[2].minkowskiDifference;
        glm::vec3 toOrigin = -currentSimplex[2].minkowskiDifference;
        
        // Calculate the normal of the triangle face.
        glm::vec3 faceNormal = glm::cross(edgeBC, edgeAC);

        // Check if the origin is outside the edge AC.
        if (glm::dot(glm::cross(faceNormal, edgeAC), toOrigin) > 0.0f)
        {
            // The origin is closer to edge AC, so remove point B and move towards the origin.
            currentSimplex.erase(currentSimplex.begin() + 1);
            currentDirection = glm::cross(glm::cross(edgeAC, toOrigin), edgeAC);
        }
        else
        {
            // Check if the origin is outside the edge BC.
            if (glm::dot(glm::cross(edgeBC, faceNormal), toOrigin) > 0.0f)
            {
                // The origin is closer to edge BC, so remove point A and move towards the origin.
                currentSimplex.erase(currentSimplex.begin());
                currentDirection = glm::cross(glm::cross(edgeBC, toOrigin), edgeBC);
            }
            else
            {
                // The origin is above or below the triangle face.
                if (glm::dot(faceNormal, toOrigin) > 0.0f)
                {
                    currentDirection = faceNormal;
                }
                else
                {
                    // Winding order needs adjustment to keep the normal pointing towards the origin.
                    std::swap(currentSimplex[0], currentSimplex[1]);
                    currentDirection = -faceNormal;
                }
            }
        }
        return false;
    }

    // Logic for a tetrahedron simplex (four points).
    if (currentSimplex.size() == 4)
    {
        // Calculate edges from the latest point D added to the simplex.
        glm::vec3 edgeDA = currentSimplex[0].minkowskiDifference - currentSimplex[3].minkowskiDifference;
        glm::vec3 edgeDB = currentSimplex[1].minkowskiDifference - currentSimplex[3].minkowskiDifference;
        glm::vec3 edgeDC = currentSimplex[2].minkowskiDifference - currentSimplex[3].minkowskiDifference;
        glm::vec3 toOrigin = -currentSimplex[3].minkowskiDifference;

        // Calculate normals for the three faces connected to point D.
        glm::vec3 normalABC = glm::cross(edgeDB, edgeDA);
        glm::vec3 normalACD = glm::cross(edgeDA, edgeDC);
        glm::vec3 normalADB = glm::cross(edgeDC, edgeDB);

        // If the origin is in front of face ABC, remove point C and continue.
        if (glm::dot(normalABC, toOrigin) > 0.0f)
        {
            currentSimplex.erase(currentSimplex.begin() + 2);
            currentDirection = normalABC;
            return false;
        }

        // If the origin is in front of face ACD, remove point B and continue.
        if (glm::dot(normalACD, toOrigin) > 0.0f)
        {
            currentSimplex.erase(currentSimplex.begin() + 1);
            currentDirection = normalACD;
            return false;
        }

        // If the origin is in front of face ADB, remove point A and continue.
        if (glm::dot(normalADB, toOrigin) > 0.0f)
        {
            currentSimplex.erase(currentSimplex.begin());
            currentDirection = normalADB;
            return false;
        }

        // If the origin is not in front of any face, it is inside the tetrahedron.
        return true;
    }

    return false;
}

void Physics3D::CalculatePenetrationEPA(const GjkShape& shapeA, const GjkShape& shapeB, std::vector<SupportPoint>& currentSimplex, glm::vec3& outNormal, float& outDepth, glm::vec3& outContactPoint)
{
    // The Expanding Polytope Algorithm (EPA) is used to find the minimum penetration depth and collision normal after GJK detects an intersection.
    // It iteratively expands the simplex (the polytope) until the closest face to the origin is reached on the boundary of the Minkowski Difference.
    std::vector<SupportPoint> polytope = currentSimplex;

    // Define the initial four faces of the tetrahedron using indices of the simplex points.
    // The faces must be defined with consistent winding order to ensure normals point outwards.
    std::vector<int> faceIndices =
    {
        0, 1, 2,
        0, 3, 1,
        0, 2, 3,
        1, 3, 2
    };

    // Calculate the outward-pointing normal of a triangle face using the cross product of its edges.
    auto getFaceNormal = [&](int a, int b, int c)
    {
        glm::vec3 edgeAB = polytope[b].minkowskiDifference - polytope[a].minkowskiDifference;
        glm::vec3 edgeAC = polytope[c].minkowskiDifference - polytope[a].minkowskiDifference;
        return glm::normalize(glm::cross(edgeAB, edgeAC));
    };

    // Helper function to extract the exact world space contact point using barycentric coordinates.
    // It projects the origin onto the closest face and interpolates the original shape points.
    auto extractContactPoint = [&](int indexA, int indexB, int indexC, glm::vec3 normal, float depth)
    {
        glm::vec3 point0 = polytope[indexA].minkowskiDifference;
        glm::vec3 point1 = polytope[indexB].minkowskiDifference;
        glm::vec3 point2 = polytope[indexC].minkowskiDifference;

        glm::vec3 projectedOrigin = normal * depth;

        // Calculate vectors for barycentric coordinate calculation.
        glm::vec3 edge0 = point1 - point0;
        glm::vec3 edge1 = point2 - point0;
        glm::vec3 edge2 = projectedOrigin - point0;

        float dot00 = glm::dot(edge0, edge0);
        float dot01 = glm::dot(edge0, edge1);
        float dot11 = glm::dot(edge1, edge1);
        float dot20 = glm::dot(edge2, edge0);
        float dot21 = glm::dot(edge2, edge1);

        float denominator = dot00 * dot11 - dot01 * dot01;
        float u, v, w;

        // Handle degenerate triangles to prevent division by zero errors.
        if (std::abs(denominator) < 0.0001f)
        {
            u = v = w = 1.0f / 3.0f;
        }
        else
        {
            v = (dot11 * dot20 - dot01 * dot21) / denominator;
            w = (dot00 * dot21 - dot01 * dot20) / denominator;
            u = 1.0f - v - w;
        }

        // Interpolate points from the original shapes A and B to find the true contact position.
        glm::vec3 contactOnA = u * polytope[indexA].pointOnA + v * polytope[indexB].pointOnA + w * polytope[indexC].pointOnA;
        glm::vec3 contactOnB = u * polytope[indexA].pointOnB + v * polytope[indexB].pointOnB + w * polytope[indexC].pointOnB;

        // The final contact point is the average of the two interpolated points.
        return (contactOnA + contactOnB) * 0.5f;
    };

    // Ensure all initial faces are pointing outwards from the origin to maintain a convex hull.
    for (size_t i = 0; i < faceIndices.size(); i += 3)
    {
        glm::vec3 normal = getFaceNormal(faceIndices[i], faceIndices[i + 1], faceIndices[i + 2]);
        if (glm::dot(normal, polytope[faceIndices[i]].minkowskiDifference) < 0.0f)
        {
            // Reverse the winding order to flip the normal direction.
            std::swap(faceIndices[i + 1], faceIndices[i + 2]);
        }
    }

    const int maxIterations = 32;
    const float tolerance = 0.001f;

    for (int iteration = 0; iteration < maxIterations; ++iteration)
    {
        float minDistance = FLT_MAX;
        int closestFaceIndex = 0;
        glm::vec3 closestNormal(0.0f);

        // Find the closest face of the current polytope to the origin.
        for (size_t i = 0; i < faceIndices.size(); i += 3)
        {
            glm::vec3 normal = getFaceNormal(faceIndices[i], faceIndices[i + 1], faceIndices[i + 2]);
            // The distance from the origin to the face is the dot product of the normal and any point on the face.
            float distance = glm::dot(normal, polytope[faceIndices[i]].minkowskiDifference);

            if (distance < minDistance)
            {
                minDistance = distance;
                closestFaceIndex = static_cast<int>(i);
                closestNormal = normal;
            }
        }

        // Acquire a new support point in the direction of the closest face to further expand the polytope.
        SupportPoint newSupport = GetSupport(shapeA, shapeB, closestNormal);
        float supportDistance = glm::dot(closestNormal, newSupport.minkowskiDifference);

        // If the distance to the new support point is not significantly further than the distance to the closest face,
        // we have reached the boundary of the Minkowski Difference and found the true minimum penetration.
        if (supportDistance - minDistance < tolerance)
        {
            outNormal = closestNormal;
            outDepth = supportDistance;
            outContactPoint = extractContactPoint(faceIndices[closestFaceIndex], faceIndices[closestFaceIndex + 1], faceIndices[closestFaceIndex + 2], closestNormal, supportDistance);
            return;
        }

        // The uniqueEdges list will store the boundary edges of the "hole" created by removing visible faces.
        std::vector<std::pair<int, int>> uniqueEdges;

        // Find all faces that are visible from the new support point.
        for (size_t i = 0; i < faceIndices.size(); i += 3)
        {
            glm::vec3 normal = getFaceNormal(faceIndices[i], faceIndices[i + 1], faceIndices[i + 2]);
            glm::vec3 toNewPoint = newSupport.minkowskiDifference - polytope[faceIndices[i]].minkowskiDifference;

            // If the dot product is positive, the face is pointing towards the new point (it is visible).
            if (glm::dot(normal, toNewPoint) > 0.0f)
            {
                // Add the edges of the visible face to the unique edges list.
                // If an edge is already in the list, it is shared between two visible faces and should be removed (not a boundary edge).
                auto addEdge = [&](int a, int b)
                {
                    bool isDuplicate = false;
                    for (auto it = uniqueEdges.begin(); it != uniqueEdges.end(); ++it)
                    {
                        if ((it->first == a && it->second == b) || (it->first == b && it->second == a))
                        {
                            uniqueEdges.erase(it);
                            isDuplicate = true;
                            break;
                        }
                    }
                    if (isDuplicate == false)
                    {
                        uniqueEdges.push_back({ a, b });
                    }
                };

                addEdge(faceIndices[i], faceIndices[i + 1]);
                addEdge(faceIndices[i + 1], faceIndices[i + 2]);
                addEdge(faceIndices[i + 2], faceIndices[i]);

                // Mark the face for removal by setting its first index to -1.
                faceIndices[i] = -1;
            }
        }

        // Rebuild the face list by excluding those marked for removal.
        std::vector<int> nextFaces;
        for (size_t i = 0; i < faceIndices.size(); i += 3)
        {
            if (faceIndices[i] != -1)
            {
                nextFaces.push_back(faceIndices[i]);
                nextFaces.push_back(faceIndices[i + 1]);
                nextFaces.push_back(faceIndices[i + 2]);
            }
        }

        // Add the new support point to the polytope and create new faces connecting the boundary edges to the new point.
        int newPointIndex = static_cast<int>(polytope.size());
        polytope.push_back(newSupport);

        for (const auto& edge : uniqueEdges)
        {
            nextFaces.push_back(edge.first);
            nextFaces.push_back(edge.second);
            nextFaces.push_back(newPointIndex);
        }

        faceIndices = nextFaces;
    }

    // fallback when maximum iterations are reached to ensure a valid return value
    outNormal = getFaceNormal(faceIndices[0], faceIndices[1], faceIndices[2]);
    outDepth = glm::dot(outNormal, polytope[faceIndices[0]].minkowskiDifference);
    outContactPoint = extractContactPoint(faceIndices[0], faceIndices[1], faceIndices[2], outNormal, outDepth);
}

bool Physics3D::SweptGJK(Physics3D* body1, Physics3D* body2, float dt, CollisionResult& outResult)
{
    // The Swept GJK algorithm performs continuous collision detection by checking if two moving shapes intersect at any point during a time step.
    // This prevents "tunneling," where objects moving at high speeds pass through each other between discrete physics updates.
    
    // Calculate the relative velocity between the two bodies to simplify the problem to one moving body and one stationary body.
    glm::vec3 velocity1 = body1->GetVelocity();
    glm::vec3 velocity2 = body2->GetVelocity();
    glm::vec3 relativeVelocity = velocity1 - velocity2;

    float speed = glm::length(relativeVelocity);

    // Skip the sweep calculation if the objects are relatively stationary to save processing time.
    if (speed < 0.001f)
    {
        return false;
    }

    // Prepare world space vertices for both shapes to ensure accurate collision detection.
    static thread_local std::vector<glm::vec3> worldVertices1;
    static thread_local std::vector<glm::vec3> worldVertices2;
    worldVertices1.clear();
    worldVertices2.clear();

    GjkShape shape1;
    shape1.type = body1->GetColliderType();

    // Transform the first shape into world space.
    if (shape1.type == ColliderType3D::BOX)
    {
        glm::quat orient1 = body1->GetEnableRotationalPhysics() ? body1->GetOrientation() : glm::quat(-glm::radians(body1->GetOwner()->GetRotate3D()));
        glm::mat4 matrix1 = glm::translate(glm::mat4(1.0f), body1->GetOwner()->GetPosition()) * glm::mat4_cast(orient1);

        for (const auto& point : body1->GetCollidePolyhedron())
        {
            worldVertices1.emplace_back(glm::vec3(matrix1 * glm::vec4(point, 1.0f)));
        }
        shape1.vertices = &worldVertices1;
    }
    else if (shape1.type == ColliderType3D::SPHERE)
    {
        shape1.center = body1->GetOwner()->GetPosition();
        shape1.radius = body1->GetSphereRadius() / 2.0f;
    }

    GjkShape shape2;
    shape2.type = body2->GetColliderType();

    // Transform the second shape into world space.
    if (shape2.type == ColliderType3D::BOX)
    {
        glm::quat orient2 = body2->GetEnableRotationalPhysics() ? body2->GetOrientation() : glm::quat(-glm::radians(body2->GetOwner()->GetRotate3D()));
        glm::mat4 matrix2 = glm::translate(glm::mat4(1.0f), body2->GetOwner()->GetPosition()) * glm::mat4_cast(orient2);

        for (const auto& point : body2->GetCollidePolyhedron())
        {
            worldVertices2.emplace_back(glm::vec3(matrix2 * glm::vec4(point, 1.0f)));
        }
        shape2.vertices = &worldVertices2;
    }
    else if (shape2.type == ColliderType3D::SPHERE)
    {
        shape2.center = body2->GetOwner()->GetPosition();
        shape2.radius = body2->GetSphereRadius() / 2.0f;
    }

    // To prevent tunneling, divide the movement into several safe steps based on a maximum distance per step.
    const float safeDistanceStep = 0.5f;
    float totalDistance = speed * dt;

    int numSteps = static_cast<int>(std::ceil(totalDistance / safeDistanceStep));
    numSteps = std::max(1, std::min(numSteps, 32));

    float timeStep = dt / static_cast<float>(numSteps);
    float previousSafeTime = 0.0f;

    // Check each sub-step for a potential collision.
    for (int i = 1; i <= numSteps; ++i)
    {
        float currentTime = i * timeStep;

        // Apply temporary offsets based on velocity to simulate movement at the current time step.
        shape1.offset = velocity1 * currentTime;
        shape2.offset = velocity2 * currentTime;

        std::vector<SupportPoint> simplex;

        // Execute a discrete GJK check at the current time step.
        if (CheckCollisionGJK(shape1, shape2, simplex))
        {
            float lowTime = previousSafeTime;
            float highTime = currentTime;
            std::vector<SupportPoint> exactSimplex;

            // Once a collision is detected in a time block, use binary search to find the precise time of impact (TOI).
            for (int b = 0; b < 8; ++b)
            {
                float midTime = (lowTime + highTime) * 0.5f;

                shape1.offset = velocity1 * midTime;
                shape2.offset = velocity2 * midTime;

                std::vector<SupportPoint> midSimplex;
                if (CheckCollisionGJK(shape1, shape2, midSimplex))
                {
                    // The collision occurs before or at midTime.
                    highTime = midTime;
                    exactSimplex = midSimplex;
                }
                else
                {
                    // The collision occurs after midTime.
                    lowTime = midTime;
                }
            }

            // Finalize shape positions at the precise impact time found by binary search.
            shape1.offset = velocity1 * highTime;
            shape2.offset = velocity2 * highTime;

            glm::vec3 contactNormal(0.0f);
            float contactDepth = 0.0f;
            glm::vec3 contactPoint(0.0f);

            // Use EPA to determine the contact normal and penetration at the exact moment of impact.
            CalculatePenetrationEPA(shape1, shape2, exactSimplex, contactNormal, contactDepth, contactPoint);

            // Fill the collision result structure with the impact data.
            outResult.hasCollided = true;
            outResult.timeOfImpact = highTime / dt;
            outResult.collisionNormal = contactNormal;
            outResult.otherObject = body2->GetOwner();

            return true;
        }

        previousSafeTime = currentTime;
    }

    return false;
}


//======== Legacy: SAT ========//

/*
bool Physics3D::CollisionPPSAT(Object* obj, Object* obj2, CollisionMode mode)
{
    auto* physics1 = obj->GetComponent<Physics3D>();
    auto* physics2 = obj2->GetComponent<Physics3D>();

    if (physics1->GetCollidePolyhedron().empty() == false && physics2->GetCollidePolyhedron().empty() == false)
    {
        const auto& poly1 = physics1->GetCollidePolyhedron(); // ¸â¹ö Á÷Á¢ Á¢±Ù(collidePolyhedron) ´ë½Å Getter »ç¿ë ±ÇÀå
        const auto& poly2 = physics2->GetCollidePolyhedron();

        if (poly1.empty() || poly2.empty())
        {
            return false;
        }

        // Use thread_local static variables to prevent heap allocation on every collision check
        // .clear() sets size to 0 but retains capacity for memory reuse
        static thread_local std::vector<glm::vec3> rotatedPoly1;
        static thread_local std::vector<glm::vec3> rotatedPoly2;
        static thread_local std::vector<glm::vec3> collisionAxes;

        rotatedPoly1.clear();
        rotatedPoly2.clear();
        collisionAxes.clear();

        // Transform local polyhedron vertices to world space
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

        // Temporal Coherence: Check the cached separating axis first
        auto cacheIterator = physics1->separatingAxisCache.find(physics2);

        if (cacheIterator != physics1->separatingAxisCache.end())
        {
            glm::vec3 cachedAxis = cacheIterator->second;
            float dummyDepth, dummyMin1, dummyMax1, dummyMin2, dummyMax2;

            // If the old axis still separates them, skip all other axis calculations
            if (IsSeparatingAxis(cachedAxis, rotatedPoly1, rotatedPoly2, &dummyDepth, &dummyMin1, &dummyMax1, &dummyMin2, &dummyMax2))
            {
                return false; // Early Exit: Massive performance boost
            }
        }

        // Collect all potential separating axes (face normals and edge cross products)
        collisionAxes.push_back(glm::normalize(glm::vec3(rotationMatrix1[0])));
        collisionAxes.push_back(glm::normalize(glm::vec3(rotationMatrix1[1])));
        collisionAxes.push_back(glm::normalize(glm::vec3(rotationMatrix1[2])));
        collisionAxes.push_back(glm::normalize(glm::vec3(rotationMatrix2[0])));
        collisionAxes.push_back(glm::normalize(glm::vec3(rotationMatrix2[1])));
        collisionAxes.push_back(glm::normalize(glm::vec3(rotationMatrix2[2])));

        for (size_t i = 0; i < rotatedPoly1.size(); ++i)
        {
            for (size_t j = 0; j < rotatedPoly2.size(); ++j)
            {
                glm::vec3 edge1 = rotatedPoly1[(i + 1) % rotatedPoly1.size()] - rotatedPoly1[i];
                glm::vec3 edge2 = rotatedPoly2[(j + 1) % rotatedPoly2.size()] - rotatedPoly2[j];
                glm::vec3 crossAxis = glm::cross(edge1, edge2);

                if (glm::length(crossAxis) > 0.0001f)
                {
                    collisionAxes.push_back(glm::normalize(crossAxis));
                }
            }
        }

        float minDepth = FLT_MAX;
        glm::vec3 collisionNormal(0.f);

        // Test for separation along each axis (Separating Axis Theorem)
        for (const auto& axis : collisionAxes)
        {
            float min1, max1, min2, max2;
            float currentDepth;

            if (IsSeparatingAxis(axis, rotatedPoly1, rotatedPoly2, &currentDepth, &min1, &max1, &min2, &max2))
            {
                // Save the axis that separated the objects to the cache for the next frame
                physics1->separatingAxisCache[physics2] = axis;
                physics2->separatingAxisCache[physics1] = -axis; // Reverse direction for the other body

                return false; // Gap found, no collision
            }

            if (currentDepth < minDepth)
            {
                minDepth = currentDepth;
                collisionNormal = ((max1 - min2) < (max2 - min1)) ? axis : -axis;
            }
        }

        // Actual collision occurred. Remove from cache to recalculate accurately next time.
        physics1->separatingAxisCache.erase(physics2);
        physics2->separatingAxisCache.erase(physics1);

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

bool Physics3D::CollisionSSSAT(Object* obj, Object* obj2, CollisionMode mode)
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

bool Physics3D::CollisionPSSAT(Object* poly, Object* sph, CollisionMode mode)
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

glm::vec3 Physics3D::RotatePoint(const glm::vec3& pt, const glm::vec3& pos, const glm::quat& rot)
{
    // Rotate a point around an origin and translate it
    return (rot * pt) + pos;
}

bool Physics3D::IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> points1, const std::vector<glm::vec3> points2, float* axisDepth, float* min1, float* max1, float* min2, float* max2)
{
    // Project both polygons onto an axis to check for overlap
    float currentMin1 = FLT_MAX;
    float currentMax1 = -FLT_MAX;
    float currentMin2 = FLT_MAX;
    float currentMax2 = -FLT_MAX;

    for (const glm::vec3& vertex : points1)
    {
        float projection = glm::dot(axis, vertex);
        currentMin1 = std::min(currentMin1, projection);
        currentMax1 = std::max(currentMax1, projection);
    }

    for (const glm::vec3& vertex : points2)
    {
        float projection = glm::dot(axis, vertex);
        currentMin2 = std::min(currentMin2, projection);
        currentMax2 = std::max(currentMax2, projection);
    }

    // Determine the amount of overlap along the axis
    *axisDepth = std::min(currentMax2 - currentMin1, currentMax1 - currentMin2);
    *min1 = currentMin1;
    *max1 = currentMax1;
    *min2 = currentMin2;
    *max2 = currentMax2;

    return !(currentMax1 >= currentMin2 && currentMax2 >= currentMin1);
}

bool Physics3D::IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> pointsPoly, const glm::vec3 pointSphere, const float radius, float* axisDepth, float* min1, float* max1, float* min2, float* max2)
{
    // Project polygon and sphere (represented as center +/- radius) onto an axis
    float currentMin1 = FLT_MAX;
    float currentMax1 = -FLT_MAX;

    for (const glm::vec3& vertex : pointsPoly)
    {
        float projection = glm::dot(vertex, axis);
        currentMin1 = std::min(currentMin1, projection);
        currentMax1 = std::max(currentMax1, projection);
    }

    float sphereProjection = glm::dot(pointSphere, axis);
    float currentMin2 = sphereProjection - radius;
    float currentMax2 = sphereProjection + radius;

    *axisDepth = std::min(currentMax2 - currentMin1, currentMax1 - currentMin2);
    *min1 = currentMin1;
    *max1 = currentMax1;
    *min2 = currentMin2;
    *max2 = currentMax2;

    return !(currentMax1 >= currentMin2 && currentMax2 >= currentMin1);
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

bool Physics3D::StaticSATIntersection(Physics3D*, Physics3D*, const std::vector<glm::vec3>& rp1, const std::vector<glm::vec3>& rp2, const glm::mat4& rm1, const glm::mat4& rm2, glm::vec3& oNorm, float& oDepth)
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
*/