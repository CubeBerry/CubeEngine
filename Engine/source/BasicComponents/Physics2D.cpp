//Author: DOYEONG LEE
//Project: CubeEngine
//File: Physics2D.cpp

#include "BasicComponents/Physics2D.hpp"
#include "BasicComponents/DynamicSprite.hpp"
#include <glm/geometric.hpp>
#include "Engine.hpp"

Physics2D::~Physics2D()
{
    // Remove this body from the physics manager list
    Engine::GetPhysicsManager().RemoveBody2D(this);

#ifdef _DEBUG
    // Clean up debug point sprites
    for (auto& v : points)
    {
        delete v.sprite;
    }
    points.clear();
#endif
}

void Physics2D::Init()
{
    // Register the component to the engine's physics manager
    Engine::GetPhysicsManager().AddBody2D(this);
}

void Physics2D::SetMomentOfInertia(float i)
{
    momentOfInertia = i;
    // Pre-calculate inverse inertia to avoid division during simulation
    if (i > 0.0f)
    {
        inverseInertia = 1.0f / i;
    }
    else
    {
        // Infinite inertia represents an object that cannot rotate
        inverseInertia = 0.0f;
    }
}

void Physics2D::Update(float dt)
{
    if (isGravityOn)
    {
        Gravity(dt);
    }

    // Integrate force into velocity: a = F/m, v = v + a*dt
    acceleration = force / mass;
    velocity += acceleration * dt;

    // Reset accumulated force after integration
    force = { 0.f, 0.f };

    if (friction > 0.f)
    {
        if (isGravityOn)
        {
            // Apply horizontal friction only when gravity is pulling the object down
            velocity.x *= (1.f - friction * dt);
        }
        else
        {
            // Apply full linear damping
            velocity *= (1.f - friction * dt);
        }
    }

    // Clamp velocity to maximum allowed speed
    if (std::abs(velocity.y) > velocityMax.y)
    {
        velocity.y = velocityMax.y * ((velocity.y < 0.f) ? -1.f : 1.f);
    }
    if (std::abs(velocity.x) > velocityMax.x)
    {
        velocity.x = velocityMax.x * ((velocity.x < 0.f) ? -1.f : 1.f);
    }

    // Clamp velocity to zero if below minimum threshold
    if (std::abs(velocity.x) < velocityMin.x)
    {
        velocity.x = 0.f;
    }
    if (std::abs(velocity.y) < velocityMin.y)
    {
        velocity.y = 0.f;
    }

    if (enableRotationalPhysics && inverseInertia > 0.0f)
    {
        // Calculate angular acceleration: alpha = torque / I
        glm::vec2 angularAcceleration = torque * inverseInertia;
        angularVelocity += angularAcceleration * dt;

        if (friction > 0.f)
        {
            // Apply angular damping
            angularVelocity *= (1.f - friction * dt);
        }
        torque = { 0.f, 0.f };

        // Update the owner's rotation based on angular velocity
        float currentRot = GetOwner()->GetRotate();
        GetOwner()->SetRotate(currentRot + angularVelocity.x * dt);
    }
}

void Physics2D::Gravity(float dt)
{
    if (isGravityOn == true)
    {
        // Add gravitational acceleration to vertical velocity
        velocity.y -= gravity * dt;
        if (std::abs(velocity.y) > velocityMax.y)
        {
            velocity.y = velocityMax.y * ((velocity.y < 0.f) ? -1.f : 1.f);
        }
    }
}

bool Physics2D::CheckCollision(Object* obj)
{
    // Dispatch to the correct collision detection routine based on collider types
    switch (collideType)
    {
    case CollideType::POLYGON:
    {
        if (obj->GetComponent<Physics2D>()->GetCollideType() == CollideType::POLYGON)
        {
            return CollisionPP(GetOwner(), obj);
        }
        else if (obj->GetComponent<Physics2D>()->GetCollideType() == CollideType::CIRCLE)
        {
            return CollisionPC(GetOwner(), obj);
        }
        break;
    }
    case CollideType::CIRCLE:
    {
        if (obj->GetComponent<Physics2D>()->GetCollideType() == CollideType::POLYGON)
        {
            return CollisionPC(obj, GetOwner());
        }
        else if (obj->GetComponent<Physics2D>()->GetCollideType() == CollideType::CIRCLE)
        {
            return CollisionCC(GetOwner(), obj);
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

bool Physics2D::CollisionPP(Object* obj, Object* obj2)
{
    // Polygon vs Polygon collision using Separating Axis Theorem (SAT)
    if (obj->GetComponent<Physics2D>()->GetCollidePolygon().empty() == false &&
        obj2->GetComponent<Physics2D>()->GetCollidePolygon().empty() == false)
    {
        std::vector<glm::vec2> rotatedPoints1;
        std::vector<glm::vec2> rotatedPoints2;
        float depth = INFINITY;
        glm::vec2 normal = { 0.f, 0.f };

        float min1 = INFINITY;
        float min2 = INFINITY;
        float max1 = -INFINITY;
        float max2 = -INFINITY;

        // Transform local vertices to world space based on object transform
        for (const glm::vec2 point : collidePolygon)
        {
            rotatedPoints1.push_back(RotatePoint(glm::vec2(obj->GetPosition()), point, DegreesToRadians(obj->GetRotate())));
        }

        for (const glm::vec2 point : obj2->GetComponent<Physics2D>()->GetCollidePolygon())
        {
            rotatedPoints2.push_back(RotatePoint(glm::vec2(obj2->GetPosition()), point, DegreesToRadians(obj2->GetRotate())));
        }

        // Test separating axes for the first polygon's edges
        for (size_t i = 0; i < rotatedPoints1.size(); ++i)
        {
            float axisDepth = 0.f;
            glm::vec2 edge = rotatedPoints1[(i + 1) % rotatedPoints1.size()] - rotatedPoints1[i];
            glm::vec2 axis = glm::vec2(-edge.y, edge.x); // Perpendicular vector (Normal)
            axis = normalize(axis);

            if (IsSeparatingAxis(axis, rotatedPoints1, rotatedPoints2, &axisDepth, &min1, &max1, &min2, &max2))
            {
                return false; // Gap found, no collision
            }
            if (axisDepth < depth)
            {
                depth = axisDepth;
                normal = ((max1 - min2) < (max2 - min1)) ? axis : -axis;
            }
        }

        // Test separating axes for the second polygon's edges
        for (size_t i = 0; i < rotatedPoints2.size(); ++i)
        {
            float axisDepth = 0.f;
            glm::vec2 edge = rotatedPoints2[(i + 1) % rotatedPoints2.size()] - rotatedPoints2[i];
            glm::vec2 axis = glm::vec2(-edge.y, edge.x);
            axis = normalize(axis);

            if (IsSeparatingAxis(axis, rotatedPoints1, rotatedPoints2, &axisDepth, &min1, &max1, &min2, &max2))
            {
                return false;
            }
            if (axisDepth < depth)
            {
                depth = axisDepth;
                normal = ((max1 - min2) < (max2 - min1)) ? axis : -axis;
            }
        }

        // Penetration resolution and collision response
        if (obj->GetComponent<Physics2D>()->GetIsGhostCollision() == false &&
            obj2->GetComponent<Physics2D>()->GetIsGhostCollision() == false)
        {
            // Apply positional correction based on body types
            if (obj->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID &&
                obj2->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID)
            {
                obj->SetXPosition(obj->GetPosition().x + (-normal * depth / 2.f).x);
                obj->SetYPosition(obj->GetPosition().y + (-normal * depth / 2.f).y);
                obj2->SetXPosition(obj2->GetPosition().x + (normal * depth / 2.f).x);
                obj2->SetYPosition(obj2->GetPosition().y + (normal * depth / 2.f).y);
            }
            else if (obj->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID)
            {
                obj->SetXPosition(obj->GetPosition().x + (-normal * depth).x);
                obj->SetYPosition(obj->GetPosition().y + (-normal * depth).y);
            }
            else if (obj2->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID)
            {
                obj2->SetXPosition(obj2->GetPosition().x + (normal * depth).x);
                obj2->SetYPosition(obj2->GetPosition().y + (normal * depth).y);
            }

            // Find contact points to apply rotational impulses
            float maxDot1 = -FLT_MAX;
            for (auto& p : rotatedPoints1)
            {
                maxDot1 = std::max(maxDot1, glm::dot(p, normal));
            }

            glm::vec2 deepest1(0.0f);
            int count1 = 0;
            for (auto& p : rotatedPoints1)
            {
                if (glm::dot(p, normal) > maxDot1 - 0.01f)
                {
                    deepest1 += p;
                    count1++;
                }
            }
            deepest1 /= static_cast<float>(count1);

            float maxDot2 = -FLT_MAX;
            for (auto& p : rotatedPoints2)
            {
                maxDot2 = std::max(maxDot2, glm::dot(p, -normal));
            }

            glm::vec2 deepest2(0.0f);
            int count2 = 0;
            for (auto& p : rotatedPoints2)
            {
                if (glm::dot(p, -normal) > maxDot2 - 0.01f)
                {
                    deepest2 += p;
                    count2++;
                }
            }
            deepest2 /= static_cast<float>(count2);

            // Determine the final contact point
            glm::vec2 contactPoint;
            if (count1 < count2)
            {
                contactPoint = deepest1;
            }
            else if (count2 < count1)
            {
                contactPoint = deepest2;
            }
            else
            {
                contactPoint = (deepest1 + deepest2) * 0.5f;
            }

            // Calculate and apply linear/angular impulses
            CalculateLinearVelocity(*obj->GetComponent<Physics2D>(), *obj2->GetComponent<Physics2D>(), normal, &depth, contactPoint);
        }
        return true;
    }
    return false;
}

bool Physics2D::CollisionCC(Object* obj, Object* obj2)
{
    // Circle vs Circle distance-based collision check
    float distanceX = obj->GetPosition().x - obj2->GetPosition().x;
    float distanceY = obj->GetPosition().y - obj2->GetPosition().y;
    float distance = std::sqrt(distanceX * distanceX + distanceY * distanceY);

    float depth = INFINITY;
    glm::vec2 normal = { 0.f, 0.f };

    float radiusSum = obj->GetComponent<Physics2D>()->GetCircleCollideRadius() + obj2->GetComponent<Physics2D>()->GetCircleCollideRadius();

    if (distance <= radiusSum)
    {
        float unitX = distanceX / distance;
        float unitY = distanceY / distance;

        // Calculate overlap amount
        float overlap = (radiusSum - distance) / 2.0f;
        normal = normalize(glm::vec2(obj2->GetPosition()) - glm::vec2(obj->GetPosition()));
        depth = overlap - distance;

        if (obj->GetComponent<Physics2D>()->GetIsGhostCollision() == false &&
            obj2->GetComponent<Physics2D>()->GetIsGhostCollision() == false)
        {
            // Position correction
            if (obj->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID &&
                obj2->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID)
            {
                obj->SetXPosition(obj->GetPosition().x + (overlap * unitX));
                obj->SetYPosition(obj->GetPosition().y + (overlap * unitY));
                obj2->SetXPosition(obj2->GetPosition().x - (overlap * unitX));
                obj2->SetYPosition(obj2->GetPosition().y - (overlap * unitY));
            }
            else if (obj->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID)
            {
                obj->SetXPosition(obj->GetPosition().x + (overlap * 2.0f * unitX));
                obj->SetYPosition(obj->GetPosition().y + (overlap * 2.0f * unitY));
            }
            else if (obj2->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID)
            {
                obj2->SetXPosition(obj2->GetPosition().x - (overlap * 2.0f * unitX));
                obj2->SetYPosition(obj2->GetPosition().y - (overlap * 2.0f * unitY));
            }

            // Resolve impulse at the point of contact
            glm::vec2 contactPoint = glm::vec2(obj->GetPosition()) + normal * obj->GetComponent<Physics2D>()->GetCircleCollideRadius();
            CalculateLinearVelocity(*obj->GetComponent<Physics2D>(), *obj2->GetComponent<Physics2D>(), -normal, &depth, contactPoint);
        }
        return true;
    }
    return false;
}

bool Physics2D::CollisionPC(Object* poly, Object* cir)
{
    // Polygon vs Circle collision check
    glm::vec2 circleCenter = cir->GetPosition();
    float circleRadius = cir->GetComponent<Physics2D>()->GetCircleCollideRadius();
    std::vector<glm::vec2> rotatedPoints;

    glm::vec2 normal = { 0.f,0.f };
    float depth = INFINITY;

    float min1 = INFINITY;
    float min2 = INFINITY;
    float max1 = -INFINITY;
    float max2 = -INFINITY;

    // Convert polygon vertices to world space
    for (const glm::vec2 point : collidePolygon)
    {
        rotatedPoints.push_back(RotatePoint(poly->GetPosition(), point, DegreesToRadians(poly->GetRotate())));
    }

    // Check SAT using polygon edge normals
    for (size_t i = 0; i < rotatedPoints.size(); ++i)
    {
        float axisDepth = 0.f;
        glm::vec2 edge = rotatedPoints[(i + 1) % rotatedPoints.size()] - rotatedPoints[i];
        glm::vec2 axis = glm::vec2(-edge.y, edge.x);
        axis = normalize(axis);

        if (IsSeparatingAxis(axis, rotatedPoints, circleCenter, circleRadius, &axisDepth, &min1, &max1, &min2, &max2))
        {
            return false;
        }
        if (axisDepth < depth)
        {
            depth = axisDepth;
            normal = axis;
        }
    }

    // Check additional axis from the closest vertex to circle center
    {
        glm::vec2 closestPoint = FindClosestPointOnSegment(circleCenter, rotatedPoints);
        float axisDepth = 0.f;
        glm::vec2 axis = closestPoint - circleCenter;
        axis = normalize(axis);

        if (IsSeparatingAxis(axis, rotatedPoints, circleCenter, circleRadius, &axisDepth, &min1, &max1, &min2, &max2))
        {
            return false;
        }
        if (axisDepth < depth)
        {
            depth = axisDepth;
            normal = axis;
        }
    }

    // Ensure collision normal is pointing in the correct direction
    glm::vec2 direction = FindSATCenter(rotatedPoints) - circleCenter;
    if (glm::dot(direction, normal) < 0.f)
    {
        normal = -normal;
    }

    if (poly->GetComponent<Physics2D>()->GetIsGhostCollision() == false &&
        cir->GetComponent<Physics2D>()->GetIsGhostCollision() == false)
    {
        // Resolve overlap
        if (poly->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID &&
            cir->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID)
        {
            poly->SetXPosition(poly->GetPosition().x + (normal * depth / 2.f).x);
            poly->SetYPosition(poly->GetPosition().y + (normal * depth / 2.f).y);
            cir->SetXPosition(cir->GetPosition().x + (-normal * depth / 2.f).x);
            cir->SetYPosition(cir->GetPosition().y + (-normal * depth / 2.f).y);
        }
        else if (poly->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID)
        {
            poly->SetXPosition(poly->GetPosition().x + (normal * depth).x);
            poly->SetYPosition(poly->GetPosition().y + (normal * depth).y);
        }
        else if (cir->GetComponent<Physics2D>()->GetBodyType() == BodyType::RIGID)
        {
            cir->SetXPosition(cir->GetPosition().x + (-normal * depth).x);
            cir->SetYPosition(cir->GetPosition().y + (-normal * depth).y);
        }

        // Apply impulse calculation
        glm::vec2 contactPoint = FindClosestPointOnSegment(circleCenter, rotatedPoints);
        CalculateLinearVelocity(*poly->GetComponent<Physics2D>(), *cir->GetComponent<Physics2D>(), normal, &depth, contactPoint);
    }
    return true;
}

bool Physics2D::CollisionPPWithoutPhysics(Object* obj, Object* obj2)
{
    // Simplified SAT check for collision detection only (no response)
    if (obj->GetComponent<Physics2D>()->GetCollidePolygon().empty() == false &&
        obj2->GetComponent<Physics2D>()->GetCollidePolygon().empty() == false)
    {
        std::vector<glm::vec2> rotatedPoints1;
        std::vector<glm::vec2> rotatedPoints2;
        float depth = INFINITY;
        glm::vec2 normal = { 0.f, 0.f };

        float min1 = INFINITY, min2 = INFINITY;
        float max1 = -INFINITY, max2 = -INFINITY;

        for (const glm::vec2 point : obj->GetComponent<Physics2D>()->GetCollidePolygon())
        {
            rotatedPoints1.push_back(RotatePoint(glm::vec2(obj->GetPosition()), point, DegreesToRadians(obj->GetRotate())));
        }

        for (const glm::vec2 point : obj2->GetComponent<Physics2D>()->GetCollidePolygon())
        {
            rotatedPoints2.push_back(RotatePoint(glm::vec2(obj2->GetPosition()), point, DegreesToRadians(obj2->GetRotate())));
        }

        for (size_t i = 0; i < rotatedPoints1.size(); ++i)
        {
            float axisDepth = 0.f;
            glm::vec2 edge = rotatedPoints1[(i + 1) % rotatedPoints1.size()] - rotatedPoints1[i];
            glm::vec2 axis = normalize(glm::vec2(-edge.y, edge.x));
            if (IsSeparatingAxis(axis, rotatedPoints1, rotatedPoints2, &axisDepth, &min1, &max1, &min2, &max2))
            {
                return false;
            }
            if (axisDepth < depth)
            {
                depth = axisDepth;
                normal = axis;
            }
        }

        for (size_t i = 0; i < rotatedPoints2.size(); ++i)
        {
            float axisDepth = 0.f;
            glm::vec2 edge = rotatedPoints2[(i + 1) % rotatedPoints2.size()] - rotatedPoints2[i];
            glm::vec2 axis = normalize(glm::vec2(-edge.y, edge.x));
            if (IsSeparatingAxis(axis, rotatedPoints1, rotatedPoints2, &axisDepth, &min1, &max1, &min2, &max2))
            {
                return false;
            }
            if (axisDepth < depth)
            {
                depth = axisDepth;
                normal = axis;
            }
        }

        return true;
    }
    return false;
}

void Physics2D::AddCollideCircle(float r)
{
#ifdef _DEBUG
    // Generate vertex points for debug visualization of the circle
    float increment = 3.14f * 2.f / 10.f;
    for (int i = 0; i < 10; i++)
    {
        AddPoint({ r * static_cast<float>(cos(i * increment - (3.14f / 2.f))),
                    r * static_cast<float>(sin(i * increment - (3.14f / 2.f))) });
    }
#endif
    collideType = CollideType::CIRCLE;
    collidePolygon.clear();
    circle.radius = r;
}

void Physics2D::AddCollidePolygon(glm::vec2 position)
{
#ifdef _DEBUG
    AddPoint(position);
#endif
    collideType = CollideType::POLYGON;
    collidePolygon.push_back(position);
}

void Physics2D::AddCollidePolygonAABB(glm::vec2 min, glm::vec2 max)
{
#ifdef _DEBUG
    AddPoint({ min.x, min.y });
    AddPoint({ min.x, max.y });
    AddPoint({ max.x, max.y });
    AddPoint({ max.x, min.y });
#endif
    collideType = CollideType::POLYGON;
    collidePolygon.clear();
    collidePolygon = { {min.x, min.y}, {min.x, max.y}, {max.x, max.y}, {max.x, min.y} };
}

void Physics2D::AddCollidePolygonAABB(glm::vec2 size)
{
#ifdef _DEBUG
    AddPoint({ -size.x, -size.y });
    AddPoint({ -size.x, size.y });
    AddPoint({ size.x, size.y });
    AddPoint({ size.x, -size.y });
#endif
    collideType = CollideType::POLYGON;
    collidePolygon.clear();
    collidePolygon = { {-size.x, -size.y}, {-size.x, size.y}, {size.x, size.y}, {size.x, -size.y} };
}

glm::vec2 Physics2D::FindSATCenter(const std::vector<glm::vec2>& points)
{
    // Find the centroid of the polygon for consistent direction calculations
    float totalArea = 0.0f;
    float centerX = 0.0f;
    float centerY = 0.0f;

    for (size_t i = 0; i < points.size(); ++i)
    {
        glm::vec2 currentPoint = points[i];
        glm::vec2 nextPoint = points[(i + 1) % points.size()];

        float crossProduct = (currentPoint.x * nextPoint.y) - (nextPoint.x * currentPoint.y);
        totalArea += crossProduct;
        centerX += (currentPoint.x + nextPoint.x) * crossProduct;
        centerY += (currentPoint.y + nextPoint.y) * crossProduct;
    }

    centerX *= 1.f / (3.f * totalArea);
    centerY *= 1.f / (3.f * totalArea);

    return glm::vec2(centerX, centerY);
}

glm::vec2 Physics2D::FindClosestPointOnSegment(const glm::vec2& circleCenter, std::vector<glm::vec2>& vertices)
{
    // Find the point on the polygon's edges closest to the circle center
    glm::vec2 resultPoint = { 0.f, 0.f };
    float minDistanceSquared = FLT_MAX;

    for (size_t i = 0; i < vertices.size(); i++)
    {
        glm::vec2 va = vertices[i];
        glm::vec2 vb = vertices[(i + 1) % vertices.size()];
        glm::vec2 edge = vb - va;
        glm::vec2 toCircle = circleCenter - va;

        float edgeLengthSq = LengthSquared(edge);
        float t = 0.0f;
        if (edgeLengthSq > 0.0f)
        {
            // Project the circle center onto the line segment to find parameter t
            t = glm::dot(toCircle, edge) / edgeLengthSq;
            t = std::max(0.0f, std::min(1.0f, t));
        }

        glm::vec2 closestPoint = va + edge * t;
        float distSq = DistanceSquared(closestPoint, circleCenter);

        if (distSq < minDistanceSquared)
        {
            minDistanceSquared = distSq;
            resultPoint = closestPoint;
        }
    }
    return resultPoint;
}

float Physics2D::calculatePolygonRadius(const std::vector<glm::vec2>& vertices)
{
    // Calculate the distance to the vertex furthest from the center
    float centerX = 0.0f;
    float centerY = 0.0f;
    for (const glm::vec2& vertex : vertices)
    {
        centerX += vertex.x;
        centerY += vertex.y;
    }
    centerX /= static_cast<float>(vertices.size());
    centerY /= static_cast<float>(vertices.size());

    float maxDistance = 0.0f;
    for (const glm::vec2& vertex : vertices)
    {
        float distance = std::sqrt((vertex.x - centerX) * (vertex.x - centerX) + (vertex.y - centerY) * (vertex.y - centerY));
        if (distance > maxDistance)
        {
            maxDistance = distance;
        }
    }
    return maxDistance;
}

float Physics2D::DegreesToRadians(float degrees)
{
    return degrees * 3.14f / 180.f;
}

glm::vec2 Physics2D::RotatePoint(const glm::vec2 point, const glm::vec2 size, float angle)
{
    // Apply 2D rotation matrix around a point
    float x = point.x + (size.x * cos(angle) - size.y * sin(angle));
    float y = point.y + (size.x * sin(angle) + size.y * cos(angle));
    return glm::vec2(x, y);
}

bool Physics2D::IsSeparatingAxis(const glm::vec2 axis, const std::vector<glm::vec2> points1, const std::vector<glm::vec2> points2, float* axisDepth, float* min1, float* max1, float* min2, float* max2)
{
    // Project all points of both polygons onto the given axis
    float minPoint1 = INFINITY;
    float minPoint2 = INFINITY;
    float maxPoint1 = -INFINITY;
    float maxPoint2 = -INFINITY;

    for (const glm::vec2 point : points1)
    {
        float projection = glm::dot(point, axis);
        minPoint1 = std::min(minPoint1, projection);
        maxPoint1 = std::max(maxPoint1, projection);
    }

    for (const glm::vec2 point : points2)
    {
        float projection = glm::dot(point, axis);
        minPoint2 = std::min(minPoint2, projection);
        maxPoint2 = std::max(maxPoint2, projection);
    }

    // Check if the two projected ranges overlap
    *axisDepth = std::min(maxPoint2 - minPoint1, maxPoint1 - minPoint2);
    *min1 = minPoint1;
    *max1 = maxPoint1;
    *min2 = minPoint2;
    *max2 = maxPoint2;

    // Return true if there is a gap (separation)
    return !(maxPoint1 >= minPoint2 && maxPoint2 >= minPoint1);
}

bool Physics2D::IsSeparatingAxis(const glm::vec2 axis, const std::vector<glm::vec2> pointsPoly, const glm::vec2 pointCir, const float radius, float* axisDepth, float* min1, float* max1, float* min2, float* max2)
{
    // SAT projection for polygon vs circle
    float minPoint1 = INFINITY;
    float maxPoint1 = -INFINITY;

    for (const glm::vec2 point : pointsPoly)
    {
        float projection = glm::dot(point, axis);
        minPoint1 = std::min(minPoint1, projection);
        maxPoint1 = std::max(maxPoint1, projection);
    }

    // For circle, the projection range is center +/- radius
    glm::vec2 direction = normalize(axis);
    glm::vec2 directionAndRadius = direction * radius;

    float p1 = glm::dot(pointCir + directionAndRadius, axis);
    float p2 = glm::dot(pointCir - directionAndRadius, axis);

    float minPoint2 = std::min(p1, p2);
    float maxPoint2 = std::max(p1, p2);

    *axisDepth = std::min(maxPoint2 - minPoint1, maxPoint1 - minPoint2);
    *min1 = minPoint1;
    *max1 = maxPoint1;
    *min2 = minPoint2;
    *max2 = maxPoint2;

    return !(maxPoint1 >= minPoint2 && maxPoint2 >= minPoint1);
}

void Physics2D::CalculateLinearVelocity(Physics2D& body, Physics2D& body2, glm::vec2 normal, float* /*axisDepth*/, glm::vec2 contactPoint)
{
    // Lever arms from centers of mass to contact point
    glm::vec2 ra = contactPoint - glm::vec2(body.GetOwner()->GetPosition());
    glm::vec2 rb = contactPoint - glm::vec2(body2.GetOwner()->GetPosition());

    // Contact velocity including rotational component
    glm::vec2 vaContact = body.GetVelocity() + glm::vec2(-body.GetAngularVelocity().x * ra.y, body.GetAngularVelocity().x * ra.x);
    glm::vec2 vbContact = body2.GetVelocity() + glm::vec2(-body2.GetAngularVelocity().x * rb.y, body2.GetAngularVelocity().x * rb.x);

    glm::vec2 relativeVelocity = vbContact - vaContact;

    // Do not resolve if objects are moving apart
    float velAlongNormal = glm::dot(relativeVelocity, normal);
    if (velAlongNormal > 0.0f)
    {
        return;
    }

    // Determine impulse scalar magnitude
    float res = std::min(body.GetRestitution(), body2.GetRestitution());

    // 2D Cross product equivalent for rotational effect
    float raCrossN = ra.x * normal.y - ra.y * normal.x;
    float rbCrossN = rb.x * normal.y - rb.y * normal.x;

    float invMassSum = (body.GetBodyType() == BodyType::RIGID ? (1.f / body.mass) : 0.0f) +
        (body2.GetBodyType() == BodyType::RIGID ? (1.f / body2.mass) : 0.0f);

    float denominator = invMassSum;
    if (body.GetBodyType() == BodyType::RIGID && body.GetEnableRotationalPhysics())
    {
        denominator += (raCrossN * raCrossN) * body.GetInverseInertia();
    }
    if (body2.GetBodyType() == BodyType::RIGID && body2.GetEnableRotationalPhysics())
    {
        denominator += (rbCrossN * rbCrossN) * body2.GetInverseInertia();
    }

    if (denominator <= 0.0f)
    {
        return;
    }

    // Apply the impulse to both bodies
    float j = -(1.f + res) * velAlongNormal / denominator;
    glm::vec2 impulse = normal * j;

    if (body.GetBodyType() == BodyType::RIGID)
    {
        body.SetVelocity(body.GetVelocity() - impulse * (1.f / body.mass));
        if (body.GetEnableRotationalPhysics())
        {
            body.SetAngularVelocity(glm::vec2(body.GetAngularVelocity().x - raCrossN * j * body.GetInverseInertia(), 0.0f));
        }
    }
    if (body2.GetBodyType() == BodyType::RIGID)
    {
        body2.SetVelocity(body2.GetVelocity() + impulse * (1.f / body2.mass));
        if (body2.GetEnableRotationalPhysics())
        {
            body2.SetAngularVelocity(glm::vec2(body2.GetAngularVelocity().x + rbCrossN * j * body2.GetInverseInertia(), 0.0f));
        }
    }
}

#ifdef _DEBUG
void Physics2D::AddPoint(glm::vec2 /*pos*/)
{
    //if (points.empty() == true)
    //{
    //	Point temp;
    //	temp.pos = { 0.f,0.f };
    //	temp.sprite = new Sprite();
    //	temp.sprite->AddQuad({ 1.f,0.f,0.f,1.f });
    //	points.push_back(std::move(temp));
    //}
    //Point temp;
    //temp.pos = pos;
    //temp.sprite = new Sprite();
    //switch (bodyType)
    //{
    //case BodyType::RIGID:
    //	temp.sprite->AddQuad({ 0.f,1.f,0.f,1.f });
    //	break;
    //case BodyType::BLOCK:
    //	temp.sprite->AddQuad({ 0.f,0.f,1.f,1.f });
    //	break;
    //default:
    //	temp.sprite->AddQuad({ 1.f,1.f,1.f,1.f });
    //	break;
    //}
    //points.push_back(std::move(temp));
}
#endif
