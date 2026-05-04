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
    if (std::find(bodies2D.begin(), bodies2D.end(), body) == bodies2D.end())
    {
        bodies2D.push_back(body);

        if (bvhNodes2D.empty())
        {
            int initialCapacity = 256;
            bvhNodes2D.resize(initialCapacity);
            for (int i = 0; i < initialCapacity - 1; ++i)
            {
                bvhNodes2D[i].nextFreeNodeIndex = i + 1;
            }
            bvhNodes2D[initialCapacity - 1].nextFreeNodeIndex = -1;
            freeListIndex2D = 0;
        }

        int leafIndex = AllocateNode2D();
        bvhNodes2D[leafIndex].physicsBody = body;

        AABB2D tightAABB = ComputeAABB2D(body);
        bvhNodes2D[leafIndex].boundingBox.minExtent = tightAABB.minExtent - FAT_BOUNDS_MARGIN_2D;
        bvhNodes2D[leafIndex].boundingBox.maxExtent = tightAABB.maxExtent + FAT_BOUNDS_MARGIN_2D;

        InsertLeaf2D(leafIndex);
        bodyNodeMap2D[body] = leafIndex;
    }
}

void PhysicsManager::RemoveBody2D(Physics2D* body)
{
    auto iterator = std::find(bodies2D.begin(), bodies2D.end(), body);
    if (iterator != bodies2D.end())
    {
        bodies2D.erase(iterator);

        // Prevent dangling pointers in Narrow Phase cache
        for (Physics2D* remainingBody : bodies2D)
        {
            remainingBody->RemoveFromCollisionCache(body);
        }

        if (bodyNodeMap2D.find(body) != bodyNodeMap2D.end())
        {
            int leafIndex = bodyNodeMap2D[body];
            RemoveLeaf2D(leafIndex);
            FreeNode2D(leafIndex);
            bodyNodeMap2D.erase(body);
        }
    }
}

void PhysicsManager::AddBody3D(Physics3D* body)
{
    if (std::find(bodies3D.begin(), bodies3D.end(), body) == bodies3D.end())
    {
        bodies3D.push_back(body);

        // Initialize node pool if empty
        if (bvhNodes.empty())
        {
            int initialCapacity = 256;
            bvhNodes.resize(initialCapacity);
            for (int i = 0; i < initialCapacity - 1; ++i)
            {
                bvhNodes[i].nextFreeNodeIndex = i + 1;
            }
            bvhNodes[initialCapacity - 1].nextFreeNodeIndex = -1;
            freeListIndex = 0;
        }

        // Allocate a leaf node for the new body
        int leafIndex = AllocateNode();
        bvhNodes[leafIndex].physicsBody = body;

        // Calculate initial AABB and add Margin (Fat AABB)
        AABB tightAABB = ComputeAABB(body);
        glm::vec3 margin(0.2f, 0.2f, 0.2f); // Fat margin to prevent frequent updates
        bvhNodes[leafIndex].boundingBox.minExtent = tightAABB.minExtent - FAT_BOUNDS_MARGIN;
        bvhNodes[leafIndex].boundingBox.maxExtent = tightAABB.maxExtent + FAT_BOUNDS_MARGIN;

        InsertLeaf(leafIndex);
        bodyNodeMap[body] = leafIndex;
    }
}

void PhysicsManager::RemoveBody3D(Physics3D* body)
{
    auto iterator = std::find(bodies3D.begin(), bodies3D.end(), body);
    if (iterator != bodies3D.end())
    {
        bodies3D.erase(iterator);

        // Clean up cached pointers in all other bodies to prevent dangling pointers
        for (Physics3D* remainingBody : bodies3D)
        {
            remainingBody->RemoveFromCollisionCache(body);
        }

        // Remove from the dynamic tree and free the node
        if (bodyNodeMap.find(body) != bodyNodeMap.end())
        {
            int leafIndex = bodyNodeMap[body];
            RemoveLeaf(leafIndex);
            FreeNode(leafIndex);
            bodyNodeMap.erase(body);
        }
    }
}

void PhysicsManager::UpdatePhysics2D(float dt)
{
    if (bodies2D.empty())
    {
        return;
    }

    // Move objects based on their current velocity. This is the first phase of the physics simulation.
    ApplyMovement2D(dt);

    // Broad Phase - Quickly filter out objects that are far apart using spatial partitioning methods.
    auto collisionPairs = BroadPhase2D(dt);

	// Narrow Phase - Perform detailed collision checks on potential pairs to get exact collision resolution.
    NarrowPhase2D(collisionPairs, dt);

    // Notify objects about potential collisions so they can handle gameplay logic.
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



std::vector<CollisionPair2D> PhysicsManager::BroadPhase2D(float dt)
{
    std::vector<CollisionPair2D> collisionPairs;

    // Check if the 2D tree root exists
    if (rootNodeIndex2D == -1)
    {
        return collisionPairs;
    }

    // Update the Dynamic BVH tree incrementally
    for (Physics2D* body : bodies2D)
    {
        // Skip if the body is not registered in the tree map
        if (bodyNodeMap2D.find(body) == bodyNodeMap2D.end())
        {
            continue;
        }

        int leafIndex = bodyNodeMap2D[body];
        AABB2D currentTightAabb = ComputeAABB2D(body);

        // Check if the current tight AABB has moved outside the cached Fat AABB
        if (bvhNodes2D[leafIndex].boundingBox.Contains(currentTightAabb) == false)
        {
            // Remove the old leaf and prepare for re-insertion
            RemoveLeaf2D(leafIndex);

            // Predict future position using velocity to reduce tree update frequency
            glm::vec2 velocityMultiplier = body->GetVelocity() * 2.0f * dt;

            AABB2D newFatAabb;
            newFatAabb.minExtent = currentTightAabb.minExtent - FAT_BOUNDS_MARGIN_2D + glm::min(glm::vec2(0.0f), velocityMultiplier);
            newFatAabb.maxExtent = currentTightAabb.maxExtent + FAT_BOUNDS_MARGIN_2D + glm::max(glm::vec2(0.0f), velocityMultiplier);

            bvhNodes2D[leafIndex].boundingBox = newFatAabb;

            // Re-insert leaf into the optimal position in the tree
            InsertLeaf2D(leafIndex);
        }
    }

    // Query the updated tree to find potential collision pairs
    for (Physics2D* body : bodies2D)
    {
        if (bodyNodeMap2D.find(body) == bodyNodeMap2D.end())
        {
            continue;
        }

        int leafIndex = bodyNodeMap2D[body];
        const AABB2D& queryAabb = bvhNodes2D[leafIndex].boundingBox;

        // Traverse tree to find overlapping AABBs O(N log N)
        QueryTree2D(rootNodeIndex2D, body, queryAabb, collisionPairs);
    }

    return collisionPairs;
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

    // Linear and angular integration. This updates positions and velocities before collision checks.
    Integrate3D(dt);

    // 3D Broad Phase using Dynamic Bounding Volume Hierarchies to find potential colliding pairs.
    auto collisionPairs = BroadPhase3D(dt);

    // 3D Narrow Phase utilizing accurate algorithms to determine exact overlap and calculate contact details.
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
    //std::vector<CollisionPair3D> pairs;

    //// Helper lambda to calculate 3D AABB with support for Continuous Collision Detection (CCD)
    //auto getAabb3D = [dt](Physics3D* body, glm::vec3& outMin, glm::vec3& outMax)
    //{
    //    Object* currentOwner = body->GetOwner();
    //    glm::vec3 position = currentOwner->GetPosition();
    //    glm::vec3 velocity = body->GetVelocity();

    //    glm::vec3 halfExtent(0.5f);
    //    const auto& polyhedron = body->GetCollidePolyhedron();

    //    // Determine half-extents for Box or Sphere
    //    if (body->GetColliderType() == ColliderType3D::BOX && polyhedron.size() >= 7)
    //    {
    //        halfExtent = (polyhedron[6] - polyhedron[0]) * 0.5f;
    //    }
    //    else if (body->GetColliderType() == ColliderType3D::SPHERE)
    //    {
    //        float radius = body->GetSphereRadius() / 2.0f;
    //        if (radius > 0.0f)
    //        {
    //            halfExtent = glm::vec3(radius);
    //        }
    //    }

    //    outMin = position - halfExtent;
    //    outMax = position + halfExtent;

    //    // If using CCD, expand the AABB to include the entire movement path
    //    if (body->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
    //    {
    //        glm::vec3 displacement = velocity * dt;
    //        outMin = glm::min(outMin, outMin + displacement);
    //        outMax = glm::max(outMax, outMax + displacement);
    //    }
    //};

    //// Check for overlaps in 3D space (XYZ)
    //for (size_t i = 0; i < bodies3D.size(); ++i)
    //{
    //    glm::vec3 minA, maxA;
    //    getAabb3D(bodies3D[i], minA, maxA);

    //    for (size_t j = i + 1; j < bodies3D.size(); ++j)
    //    {
    //        glm::vec3 minB, maxB;
    //        getAabb3D(bodies3D[j], minB, maxB);

    //        if (maxA.x < minB.x || maxB.x < minA.x)
    //        {
    //            continue;
    //        }
    //        if (maxA.y < minB.y || maxB.y < minA.y)
    //        {
    //            continue;
    //        }
    //        if (maxA.z < minB.z || maxB.z < minA.z)
    //        {
    //            continue;
    //        }

    //        pairs.push_back({ bodies3D[i], bodies3D[j] });
    //    }
    //}
    //return pairs;

    std::vector<CollisionPair3D> collisionPairs;

    if (rootNodeIndex == -1)
    {
        return collisionPairs;
    }

    // Incrementally update the Dynamic Bounding Volume Hierarchy tree to reflect any movement since the last frame. This spatial partitioning approach minimizes costly tree rebuilds.
    for (Physics3D* body : bodies3D)
    {
        if (bodyNodeMap.find(body) == bodyNodeMap.end())
        {
            continue;
        }

        int leafIndex = bodyNodeMap[body];
        AABB currentTightAABB = ComputeAABB(body);

        // Account for Continuous Collision Detection displacement by expanding the tight bounding box to cover the entire swept path for the current frame.
        if (body->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
        {
            glm::vec3 displacement = body->GetVelocity() * dt;
            currentTightAABB.minExtent = glm::min(currentTightAABB.minExtent, currentTightAABB.minExtent + displacement);
            currentTightAABB.maxExtent = glm::max(currentTightAABB.maxExtent, currentTightAABB.maxExtent + displacement);
        }

        // If the current tight bounding box has moved completely outside the cached fat bounding box, the object's position in the tree must be updated.
        if (bvhNodes[leafIndex].boundingBox.Contains(currentTightAABB) == false)
        {
            // Remove the old leaf from the tree
            RemoveLeaf(leafIndex);

            // Create a new fat bounding box that predicts the movement direction using the current velocity to reduce the frequency of tree updates.
            glm::vec3 margin(0.2f, 0.2f, 0.2f);
            glm::vec3 velocityMultiplier = body->GetVelocity() * 2.0f * dt; // Predict future position

            AABB newFatAABB;
            newFatAABB.minExtent = currentTightAABB.minExtent - margin + glm::min(glm::vec3(0.0f), velocityMultiplier);
            newFatAABB.maxExtent = currentTightAABB.maxExtent + margin + glm::max(glm::vec3(0.0f), velocityMultiplier);

            bvhNodes[leafIndex].boundingBox = newFatAABB;

            // Re-insert the leaf node into the tree at the optimal new location based on the Surface Area Heuristic cost.
            InsertLeaf(leafIndex);
        }
    }

    // Query the updated Bounding Volume Hierarchy tree to find all potential collision pairs with logarithmic time complexity.
    for (Physics3D* body : bodies3D)
    {
        if (bodyNodeMap.find(body) == bodyNodeMap.end())
        {
            continue;
        }

        // We use the Fat AABB for querying to ensure we don't miss high-speed objects
        int leafIndex = bodyNodeMap[body];
        const AABB& queryAABB = bvhNodes[leafIndex].boundingBox;

        QueryTree(rootNodeIndex, body, queryAABB, collisionPairs);
    }

    return collisionPairs;
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

        // Resolve continuous collision sub-steps if continuous detection mode is enabled for the body to prevent tunneling through geometry.
        if (bodyA->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
        {
            SolveContinuous(bodyA, dt);
        }
        if (bodyB->GetCollisionDetectionMode() == CollisionDetectionMode::CONTINUOUS)
        {
            SolveContinuous(bodyB, dt);
        }

        // Dispatch standard discrete collision logic based on the geometric shape type combination of the two colliding bodies.
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

    // User-defined or engine constants for safety to prevent infinite loops during continuous collision resolution.
    const int maxIterations = 5;
    const float skinWidth = 0.005f;

    // Sub-stepping loop for continuous collision resolution. The remaining frame time is consumed incrementally as collisions are handled.
    while (remainingTime > 0.0f && iterationCount < maxIterations)
    {
        CollisionResult collisionResult = body->FindClosestCollision(remainingTime);

        // If no impact occurred within this frame slice, the object can safely move the full remaining distance.
        if (collisionResult.hasCollided == false || collisionResult.timeOfImpact > 1.0f)
        {
            body->GetOwner()->SetPosition(body->GetOwner()->GetPosition() + body->GetVelocity() * remainingTime);
            break;
        }

        // Move the object precisely to the point of impact, subtracting a small skin width to prevent it from getting mathematically stuck inside the other object.
        float moveFraction = (collisionResult.timeOfImpact > skinWidth) ? (collisionResult.timeOfImpact - skinWidth) : 0.0f;
        body->GetOwner()->SetPosition(body->GetOwner()->GetPosition() + body->GetVelocity() * remainingTime * moveFraction);

        // Resolve the velocity impulse at the contact point using linear momentum exchange and rotational effects.
        Physics3D* otherBody = collisionResult.otherObject->GetComponent<Physics3D>();
        if (otherBody != nullptr)
        {
            glm::vec3 contactPoint = body->GetPosition() - collisionResult.collisionNormal * 0.1f;
            body->CalculateLinearVelocity(*body, *otherBody, collisionResult.collisionNormal, nullptr, contactPoint);
        }

        // Subtract the consumed time and loop again to handle potential secondary collisions that might occur during the rest of the frame.
        remainingTime -= remainingTime * moveFraction;
        iterationCount++;
    }
}

std::vector<Physics3D*> PhysicsManager::GetPotentialColliders(Physics3D* queryBody, float dt)
{
    std::vector<Physics3D*> potentialColliders;

    // If the tree is empty, there is nothing to collide with
    if (rootNodeIndex == -1)
    {
        return potentialColliders;
    }

    // Calculate the swept AABB for the moving body's full trajectory this frame
    AABB sweptAABB = ComputeAABB(queryBody);
    glm::vec3 displacement = queryBody->GetVelocity() * dt;

    sweptAABB.minExtent = glm::min(sweptAABB.minExtent, sweptAABB.minExtent + displacement);
    sweptAABB.maxExtent = glm::max(sweptAABB.maxExtent, sweptAABB.maxExtent + displacement);

    // Reuse the existing QueryTree logic to find overlapping Fat AABBs
    std::vector<CollisionPair3D> outPairs;
    QueryTree(rootNodeIndex, queryBody, sweptAABB, outPairs);

    // Extract only the other bodies from the resulting pairs
    for (const CollisionPair3D& pair : outPairs)
    {
        // Because of how QueryTree is written, pair.bodyB is the found object
        potentialColliders.push_back(pair.bodyB);
    }

    return potentialColliders;
}

int PhysicsManager::AllocateNode()
{
    // Expand the pool if the free list is empty
    if (freeListIndex == -1)
    {
        int oldCapacity = static_cast<int>(bvhNodes.size());
        int newCapacity = oldCapacity * 2;
        bvhNodes.resize(newCapacity);

        for (int i = oldCapacity; i < newCapacity - 1; ++i)
        {
            bvhNodes[i].nextFreeNodeIndex = i + 1;
        }
        bvhNodes[newCapacity - 1].nextFreeNodeIndex = -1;
        freeListIndex = oldCapacity;
    }

    // Pop a node from the free list
    int allocatedIndex = freeListIndex;
    freeListIndex = bvhNodes[allocatedIndex].nextFreeNodeIndex;

    // Reset node state
    bvhNodes[allocatedIndex].parentIndex = -1;
    bvhNodes[allocatedIndex].leftChildIndex = -1;
    bvhNodes[allocatedIndex].rightChildIndex = -1;
    bvhNodes[allocatedIndex].physicsBody = nullptr;

    return allocatedIndex;
}

void PhysicsManager::FreeNode(int nodeIndex)
{
    // Push the node back to the free list
    bvhNodes[nodeIndex].nextFreeNodeIndex = freeListIndex;
    freeListIndex = nodeIndex;
}

AABB PhysicsManager::ComputeAABB(Physics3D* body)
{
    AABB resultAabb;
    Object* currentOwner = body->GetOwner();
    glm::vec3 position = currentOwner->GetPosition();

    if (body->GetColliderType() == ColliderType3D::BOX)
    {
        const auto& polyhedron = body->GetCollidePolyhedron();

        if (polyhedron.empty())
        {
            resultAabb.minExtent = position - glm::vec3(0.5f);
            resultAabb.maxExtent = position + glm::vec3(0.5f);
            return resultAabb;
        }

        glm::quat orient = body->GetEnableRotationalPhysics() ? body->GetOrientation() : glm::quat(-glm::radians(currentOwner->GetRotate3D()));
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(orient);

        glm::vec3 minExtent(FLT_MAX);
        glm::vec3 maxExtent(-FLT_MAX);

        for (const auto& vertex : polyhedron)
        {
            glm::vec3 worldVertex = glm::vec3(transform * glm::vec4(vertex, 1.0f));
            minExtent = glm::min(minExtent, worldVertex);
            maxExtent = glm::max(maxExtent, worldVertex);
        }

        resultAabb.minExtent = minExtent;
        resultAabb.maxExtent = maxExtent;
    }
    else if (body->GetColliderType() == ColliderType3D::SPHERE)
    {
        float radius = body->GetSphereRadius() / 2.0f;

        if (radius > 0.0f)
        {
            resultAabb.minExtent = position - glm::vec3(radius);
            resultAabb.maxExtent = position + glm::vec3(radius);
        }
        else
        {
            resultAabb.minExtent = position - glm::vec3(0.5f);
            resultAabb.maxExtent = position + glm::vec3(0.5f);
        }
    }
    else
    {
        resultAabb.minExtent = position - glm::vec3(0.5f);
        resultAabb.maxExtent = position + glm::vec3(0.5f);
    }

    return resultAabb;
}

void PhysicsManager::InsertLeaf(int leafNodeIndex)
{
    if (rootNodeIndex == -1)
    {
        rootNodeIndex = leafNodeIndex;
        return;
    }

    // Find the best sibling for the new leaf using Surface Area Heuristic cost calculations.
    AABB leafAABB = bvhNodes[leafNodeIndex].boundingBox;
    int currentIndex = rootNodeIndex;

    while (bvhNodes[currentIndex].IsLeaf() == false)
    {
        int leftChild = bvhNodes[currentIndex].leftChildIndex;
        int rightChild = bvhNodes[currentIndex].rightChildIndex;

        float area = bvhNodes[currentIndex].boundingBox.GetSurfaceArea();

        AABB combinedAABB;
        combinedAABB.Merge(bvhNodes[currentIndex].boundingBox, leafAABB);
        float combinedArea = combinedAABB.GetSurfaceArea();

        // Cost of creating a new parent for this node and the new leaf
        float costCreation = 2.0f * combinedArea;

        // Minimum cost of pushing the leaf further down the tree
        float costInheritance = 2.0f * (combinedArea - area);

        // Cost of descending into left child
        float costLeft;
        AABB leftCombinedAABB;
        leftCombinedAABB.Merge(leafAABB, bvhNodes[leftChild].boundingBox);
        if (bvhNodes[leftChild].IsLeaf())
        {
            costLeft = leftCombinedAABB.GetSurfaceArea() + costInheritance;
        }
        else
        {
            float oldArea = bvhNodes[leftChild].boundingBox.GetSurfaceArea();
            float newArea = leftCombinedAABB.GetSurfaceArea();
            costLeft = (newArea - oldArea) + costInheritance;
        }

        // Cost of descending into right child
        float costRight;
        AABB rightCombinedAABB;
        rightCombinedAABB.Merge(leafAABB, bvhNodes[rightChild].boundingBox);
        if (bvhNodes[rightChild].IsLeaf())
        {
            costRight = rightCombinedAABB.GetSurfaceArea() + costInheritance;
        }
        else
        {
            float oldArea = bvhNodes[rightChild].boundingBox.GetSurfaceArea();
            float newArea = rightCombinedAABB.GetSurfaceArea();
            costRight = (newArea - oldArea) + costInheritance;
        }

        // Descend according to the minimum cost
        if (costCreation < costLeft && costCreation < costRight)
        {
            break;
        }

        if (costLeft < costRight)
        {
            currentIndex = leftChild;
        }
        else
        {
            currentIndex = rightChild;
        }
    }

    int bestSiblingIndex = currentIndex;

    // Create a new parent node for the merged hierarchy branch
    int oldParentIndex = bvhNodes[bestSiblingIndex].parentIndex;
    int newParentIndex = AllocateNode();
    bvhNodes[newParentIndex].parentIndex = oldParentIndex;
    bvhNodes[newParentIndex].boundingBox.Merge(leafAABB, bvhNodes[bestSiblingIndex].boundingBox);

    if (oldParentIndex != -1)
    {
        // The sibling was not the root
        if (bvhNodes[oldParentIndex].leftChildIndex == bestSiblingIndex)
        {
            bvhNodes[oldParentIndex].leftChildIndex = newParentIndex;
        }
        else
        {
            bvhNodes[oldParentIndex].rightChildIndex = newParentIndex;
        }
    }
    else
    {
        // The sibling was the root
        rootNodeIndex = newParentIndex;
    }

    bvhNodes[newParentIndex].leftChildIndex = bestSiblingIndex;
    bvhNodes[newParentIndex].rightChildIndex = leafNodeIndex;
    bvhNodes[bestSiblingIndex].parentIndex = newParentIndex;
    bvhNodes[leafNodeIndex].parentIndex = newParentIndex;

    // Walk back up the tree refitting AABBs to encapsulate their new children bounds
    currentIndex = bvhNodes[leafNodeIndex].parentIndex;
    while (currentIndex != -1)
    {
        int left = bvhNodes[currentIndex].leftChildIndex;
        int right = bvhNodes[currentIndex].rightChildIndex;

        bvhNodes[currentIndex].boundingBox.Merge(bvhNodes[left].boundingBox, bvhNodes[right].boundingBox);
        currentIndex = bvhNodes[currentIndex].parentIndex;
    }
}

void PhysicsManager::RemoveLeaf(int leafNodeIndex)
{
    if (leafNodeIndex == rootNodeIndex)
    {
        rootNodeIndex = -1;
        return;
    }

    int parentIndex = bvhNodes[leafNodeIndex].parentIndex;
    int grandParentIndex = bvhNodes[parentIndex].parentIndex;
    int siblingIndex;

    if (bvhNodes[parentIndex].leftChildIndex == leafNodeIndex)
    {
        siblingIndex = bvhNodes[parentIndex].rightChildIndex;
    }
    else
    {
        siblingIndex = bvhNodes[parentIndex].leftChildIndex;
    }

    if (grandParentIndex != -1)
    {
        // Connect sibling to grand parent
        if (bvhNodes[grandParentIndex].leftChildIndex == parentIndex)
        {
            bvhNodes[grandParentIndex].leftChildIndex = siblingIndex;
        }
        else
        {
            bvhNodes[grandParentIndex].rightChildIndex = siblingIndex;
        }
        bvhNodes[siblingIndex].parentIndex = grandParentIndex;
        FreeNode(parentIndex);

        // Walk back up the tree refitting AABBs
        int currentIndex = grandParentIndex;
        while (currentIndex != -1)
        {
            int left = bvhNodes[currentIndex].leftChildIndex;
            int right = bvhNodes[currentIndex].rightChildIndex;

            bvhNodes[currentIndex].boundingBox.Merge(bvhNodes[left].boundingBox, bvhNodes[right].boundingBox);
            currentIndex = bvhNodes[currentIndex].parentIndex;
        }
    }
    else
    {
        // The parent was the root
        rootNodeIndex = siblingIndex;
        bvhNodes[siblingIndex].parentIndex = -1;
        FreeNode(parentIndex);
    }
}

void PhysicsManager::QueryTree(int nodeIndex, Physics3D* queryBody, const AABB& queryAABB, std::vector<CollisionPair3D>& outPairs)
{
    if (nodeIndex == -1)
    {
        return;
    }

    // Skip if AABBs do not intersect
    if (bvhNodes[nodeIndex].boundingBox.Intersects(queryAABB) == false)
    {
        return;
    }

    if (bvhNodes[nodeIndex].IsLeaf())
    {
        // Avoid self-collision and duplicate pairs (compare memory address)
        Physics3D* foundBody = bvhNodes[nodeIndex].physicsBody;
        if (queryBody != foundBody && queryBody < foundBody)
        {
            outPairs.push_back({ queryBody, foundBody });
        }
    }
    else
    {
        QueryTree(bvhNodes[nodeIndex].leftChildIndex, queryBody, queryAABB, outPairs);
        QueryTree(bvhNodes[nodeIndex].rightChildIndex, queryBody, queryAABB, outPairs);
    }
}

int PhysicsManager::AllocateNode2D()
{
    if (freeListIndex2D == -1)
    {
        int oldCapacity = static_cast<int>(bvhNodes2D.size());
        int newCapacity = oldCapacity * 2;
        bvhNodes2D.resize(newCapacity);

        for (int i = oldCapacity; i < newCapacity - 1; ++i)
        {
            bvhNodes2D[i].nextFreeNodeIndex = i + 1;
        }
        bvhNodes2D[newCapacity - 1].nextFreeNodeIndex = -1;
        freeListIndex2D = oldCapacity;
    }

    int allocatedIndex = freeListIndex2D;
    freeListIndex2D = bvhNodes2D[allocatedIndex].nextFreeNodeIndex;

    bvhNodes2D[allocatedIndex].parentIndex = -1;
    bvhNodes2D[allocatedIndex].leftChildIndex = -1;
    bvhNodes2D[allocatedIndex].rightChildIndex = -1;
    bvhNodes2D[allocatedIndex].physicsBody = nullptr;

    return allocatedIndex;
}

void PhysicsManager::FreeNode2D(int nodeIndex)
{
    bvhNodes2D[nodeIndex].nextFreeNodeIndex = freeListIndex2D;
    freeListIndex2D = nodeIndex;
}

AABB2D PhysicsManager::ComputeAABB2D(Physics2D* body)
{
    AABB2D resultAabb;
    Object* currentOwner = body->GetOwner();
    if (!currentOwner) return resultAabb;

    glm::vec2 position = currentOwner->GetPosition();
    float angle = glm::radians(currentOwner->GetRotate());
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);

    if (body->GetCollideType() == CollideType::POLYGON)
    {
        const auto& polygon = body->GetCollidePolygon();
        if (polygon.empty())
        {
            resultAabb.minExtent = position - glm::vec2(0.5f);
            resultAabb.maxExtent = position + glm::vec2(0.5f);
            return resultAabb;
        }

        float minX = FLT_MAX, maxX = -FLT_MAX;
        float minY = FLT_MAX, maxY = -FLT_MAX;

        for (const auto& vertex : polygon)
        {
            // Apply 2D rotation and translation to each vertex to calculate correct world-space AABB
            float rx = vertex.x * cosA - vertex.y * sinA;
            float ry = vertex.x * sinA + vertex.y * cosA;
            glm::vec2 worldVertex = position + glm::vec2(rx, ry);

            minX = std::min(minX, worldVertex.x);
            maxX = std::max(maxX, worldVertex.x);
            minY = std::min(minY, worldVertex.y);
            maxY = std::max(maxY, worldVertex.y);
        }
        resultAabb.minExtent = { minX, minY };
        resultAabb.maxExtent = { maxX, maxY };
    }
    else if (body->GetCollideType() == CollideType::CIRCLE)
    {
        float radius = body->GetCircleCollideRadius();
        resultAabb.minExtent = position - glm::vec2(radius);
        resultAabb.maxExtent = position + glm::vec2(radius);
    }
    else
    {
        resultAabb.minExtent = position - glm::vec2(0.5f);
        resultAabb.maxExtent = position + glm::vec2(0.5f);
    }

    return resultAabb;
}

void PhysicsManager::InsertLeaf2D(int leafNodeIndex)
{
    if (rootNodeIndex2D == -1)
    {
        rootNodeIndex2D = leafNodeIndex;
        return;
    }

    // Find the best sibling for the new leaf using Perimeter Heuristic, which is the 2D version of the Surface Area Heuristic
    AABB2D leafAABB = bvhNodes2D[leafNodeIndex].boundingBox;
    int currentIndex = rootNodeIndex2D;

    while (bvhNodes2D[currentIndex].IsLeaf() == false)
    {
        int leftChild = bvhNodes2D[currentIndex].leftChildIndex;
        int rightChild = bvhNodes2D[currentIndex].rightChildIndex;

        float area = bvhNodes2D[currentIndex].boundingBox.GetPerimeter(); // Use perimeter instead of surface area for 2D calculations

        AABB2D combinedAABB;
        combinedAABB.Merge(bvhNodes2D[currentIndex].boundingBox, leafAABB);
        float combinedArea = combinedAABB.GetPerimeter();

        float costCreation = 2.0f * combinedArea;
        float costInheritance = 2.0f * (combinedArea - area);

        // Left child cost
        float costLeft;
        AABB2D leftCombinedAABB;
        leftCombinedAABB.Merge(leafAABB, bvhNodes2D[leftChild].boundingBox);
        if (bvhNodes2D[leftChild].IsLeaf())
        {
            costLeft = leftCombinedAABB.GetPerimeter() + costInheritance;
        }
        else
        {
            float oldArea = bvhNodes2D[leftChild].boundingBox.GetPerimeter();
            float newArea = leftCombinedAABB.GetPerimeter();
            costLeft = (newArea - oldArea) + costInheritance;
        }

        // Right child cost
        float costRight;
        AABB2D rightCombinedAABB;
        rightCombinedAABB.Merge(leafAABB, bvhNodes2D[rightChild].boundingBox);
        if (bvhNodes2D[rightChild].IsLeaf())
        {
            costRight = rightCombinedAABB.GetPerimeter() + costInheritance;
        }
        else
        {
            float oldArea = bvhNodes2D[rightChild].boundingBox.GetPerimeter();
            float newArea = rightCombinedAABB.GetPerimeter();
            costRight = (newArea - oldArea) + costInheritance;
        }

        if (costCreation < costLeft && costCreation < costRight)
        {
            break;
        }

        if (costLeft < costRight)
        {
            currentIndex = leftChild;
        }
        else
        {
            currentIndex = rightChild;
        }
    }

    int bestSiblingIndex = currentIndex;

    // Create a new parent node for the merged 2D hierarchy branch
    int oldParentIndex = bvhNodes2D[bestSiblingIndex].parentIndex;
    int newParentIndex = AllocateNode2D();

    bvhNodes2D[newParentIndex].parentIndex = oldParentIndex;
    bvhNodes2D[newParentIndex].boundingBox.Merge(leafAABB, bvhNodes2D[bestSiblingIndex].boundingBox);

    if (oldParentIndex != -1)
    {
        if (bvhNodes2D[oldParentIndex].leftChildIndex == bestSiblingIndex)
        {
            bvhNodes2D[oldParentIndex].leftChildIndex = newParentIndex;
        }
        else
        {
            bvhNodes2D[oldParentIndex].rightChildIndex = newParentIndex;
        }
    }
    else
    {
        rootNodeIndex2D = newParentIndex;
    }

    bvhNodes2D[newParentIndex].leftChildIndex = bestSiblingIndex;
    bvhNodes2D[newParentIndex].rightChildIndex = leafNodeIndex;
    bvhNodes2D[bestSiblingIndex].parentIndex = newParentIndex;
    bvhNodes2D[leafNodeIndex].parentIndex = newParentIndex;

    // Walk back up the tree refitting AABBs to match the new bounds of their children
    currentIndex = bvhNodes2D[leafNodeIndex].parentIndex;
    while (currentIndex != -1)
    {
        int left = bvhNodes2D[currentIndex].leftChildIndex;
        int right = bvhNodes2D[currentIndex].rightChildIndex;

        bvhNodes2D[currentIndex].boundingBox.Merge(bvhNodes2D[left].boundingBox, bvhNodes2D[right].boundingBox);
        currentIndex = bvhNodes2D[currentIndex].parentIndex;
    }
}

void PhysicsManager::RemoveLeaf2D(int leafNodeIndex)
{
    if (leafNodeIndex == rootNodeIndex2D)
    {
        rootNodeIndex2D = -1;
        return;
    }

    int parentIndex = bvhNodes2D[leafNodeIndex].parentIndex;
    int grandParentIndex = bvhNodes2D[parentIndex].parentIndex;
    int siblingIndex;

    if (bvhNodes2D[parentIndex].leftChildIndex == leafNodeIndex)
    {
        siblingIndex = bvhNodes2D[parentIndex].rightChildIndex;
    }
    else
    {
        siblingIndex = bvhNodes2D[parentIndex].leftChildIndex;
    }

    if (grandParentIndex != -1)
    {
        if (bvhNodes2D[grandParentIndex].leftChildIndex == parentIndex)
        {
            bvhNodes2D[grandParentIndex].leftChildIndex = siblingIndex;
        }
        else
        {
            bvhNodes2D[grandParentIndex].rightChildIndex = siblingIndex;
        }
        bvhNodes2D[siblingIndex].parentIndex = grandParentIndex;
        FreeNode2D(parentIndex);

        // Walk back up the tree refitting AABBs
        int currentIndex = grandParentIndex;
        while (currentIndex != -1)
        {
            int left = bvhNodes2D[currentIndex].leftChildIndex;
            int right = bvhNodes2D[currentIndex].rightChildIndex;

            bvhNodes2D[currentIndex].boundingBox.Merge(bvhNodes2D[left].boundingBox, bvhNodes2D[right].boundingBox);
            currentIndex = bvhNodes2D[currentIndex].parentIndex;
        }
    }
    else
    {
        rootNodeIndex2D = siblingIndex;
        bvhNodes2D[siblingIndex].parentIndex = -1;
        FreeNode2D(parentIndex);
    }
}

void PhysicsManager::QueryTree2D(int nodeIndex, Physics2D* queryBody, const AABB2D& queryAABB, std::vector<CollisionPair2D>& outPairs)
{
    if (nodeIndex == -1) 
    {
        return;
    }

    if (bvhNodes2D[nodeIndex].boundingBox.Intersects(queryAABB) == false) 
    {
        return;
    }

    if (bvhNodes2D[nodeIndex].IsLeaf())
    {
        Physics2D* foundBody = bvhNodes2D[nodeIndex].physicsBody;
        if (queryBody != foundBody && queryBody < foundBody)
        {
            outPairs.push_back({ queryBody, foundBody });
        }
    }
    else
    {
        QueryTree2D(bvhNodes2D[nodeIndex].leftChildIndex, queryBody, queryAABB, outPairs);
        QueryTree2D(bvhNodes2D[nodeIndex].rightChildIndex, queryBody, queryAABB, outPairs);
    }
}
