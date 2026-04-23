//Author: DOYEONG LEE
//Project: CubeEngine
//File: PhysicsManager.hpp
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <vector>
#include <map>
#include <utility>
#include <unordered_map>

#include "BasicComponents/Physics2D.hpp"
#include "BasicComponents/Physics3D.hpp"
#include "ObjectType.hpp"
#include "CollisionMode.hpp"

// A pair of physics bodies that may collide this frame.
struct CollisionPair2D
{
	Physics2D* bodyA = nullptr;
	Physics2D* bodyB = nullptr;
};

struct CollisionPair3D
{
	Physics3D* bodyA = nullptr;
	Physics3D* bodyB = nullptr;
};

struct AABB
{
    glm::vec3 minExtent = { 0.0f, 0.0f, 0.0f };
    glm::vec3 maxExtent = { 0.0f, 0.0f, 0.0f };

    // Check if this Axis-Aligned Bounding Box overlaps with another Axis-Aligned Bounding Box by comparing their minimum and maximum extents along all three axes
    bool Intersects(const AABB& other) const
    {
        if (maxExtent.x < other.minExtent.x || minExtent.x > other.maxExtent.x) return false;
        if (maxExtent.y < other.minExtent.y || minExtent.y > other.maxExtent.y) return false;
        if (maxExtent.z < other.minExtent.z || minExtent.z > other.maxExtent.z) return false;
        return true;
    }

    // Check if this Axis-Aligned Bounding Box fully contains another Axis-Aligned Bounding Box. This is used for the Fat Axis-Aligned Bounding Box check to avoid frequent tree updates.
    bool Contains(const AABB& other) const
    {
        bool isMinInside = minExtent.x <= other.minExtent.x && minExtent.y <= other.minExtent.y && minExtent.z <= other.minExtent.z;
        bool isMaxInside = maxExtent.x >= other.maxExtent.x && maxExtent.y >= other.maxExtent.y && maxExtent.z >= other.maxExtent.z;
        return isMinInside && isMaxInside;
    }

    // Merge two Axis-Aligned Bounding Boxes into this one by taking the minimum of their minimum extents and the maximum of their maximum extents
    void Merge(const AABB& a, const AABB& b)
    {
        minExtent = glm::min(a.minExtent, b.minExtent);
        maxExtent = glm::max(a.maxExtent, b.maxExtent);
    }

    // Surface Area Heuristic cost calculation, which estimates the probability of a ray or bounding volume intersecting this node based on its surface area
    float GetSurfaceArea() const
    {
        glm::vec3 extents = maxExtent - minExtent;
        return 2.0f * (extents.x * extents.y + extents.y * extents.z + extents.z * extents.x);
    }
};

struct BVHNode
{
    AABB boundingBox;
    Physics3D* physicsBody = nullptr;

    // Tree topology
    int parentIndex = -1;
    int leftChildIndex = -1;
    int rightChildIndex = -1;

    // Used for maintaining the free list in the object pool
    int nextFreeNodeIndex = -1;

    bool IsLeaf() const
    {
        return rightChildIndex == -1;
    }
};

struct AABB2D
{
    glm::vec2 minExtent = { 0.0f, 0.0f };
    glm::vec2 maxExtent = { 0.0f, 0.0f };

    bool Intersects(const AABB2D& other) const
    {
        if (maxExtent.x < other.minExtent.x || minExtent.x > other.maxExtent.x) return false;
        if (maxExtent.y < other.minExtent.y || minExtent.y > other.maxExtent.y) return false;
        return true;
    }

    bool Contains(const AABB2D& other) const
    {
        bool isMinInside = minExtent.x <= other.minExtent.x && minExtent.y <= other.minExtent.y;
        bool isMaxInside = maxExtent.x >= other.maxExtent.x && maxExtent.y >= other.maxExtent.y;
        return isMinInside && isMaxInside;
    }

    void Merge(const AABB2D& a, const AABB2D& b)
    {
        minExtent = glm::min(a.minExtent, b.minExtent);
        maxExtent = glm::max(a.maxExtent, b.maxExtent);
    }

    // Surface Area Heuristic cost calculation for two-dimensional space uses perimeter instead of surface area to estimate the intersection probability
    float GetPerimeter() const
    {
        glm::vec2 extents = maxExtent - minExtent;
        return 2.0f * (extents.x + extents.y);
    }
};

struct BVHNode2D
{
    AABB2D boundingBox;
    Physics2D* physicsBody = nullptr;

    int parentIndex = -1;
    int leftChildIndex = -1;
    int rightChildIndex = -1;
    int nextFreeNodeIndex = -1;

    bool IsLeaf() const
    {
        return rightChildIndex == -1;
    }
};

class PhysicsManager
{
public:
	PhysicsManager()  = default;
	~PhysicsManager() = default;

	void Update(float dt);

    void AddBody2D(Physics2D* body);
    void RemoveBody2D(Physics2D* body);

    void AddBody3D(Physics3D* body);
    void RemoveBody3D(Physics3D* body);

	void SetCollisionMode(ObjectType typeA, ObjectType typeB, CollisionMode mode);
	CollisionMode GetCollisionMode(ObjectType typeA, ObjectType typeB);


    // Query the Bounding Volume Hierarchy tree to find objects in the path of a moving body. This is used for Continuous Collision Detection.
    std::vector<Physics3D*> GetPotentialColliders(Physics3D* queryBody, float dt);

private:
	void UpdatePhysics2D(float dt);
	std::vector<CollisionPair2D> BroadPhase2D(float dt);
	void ApplyMovement2D(float dt);
	void NarrowPhase2D(std::vector<CollisionPair2D>& pairs, float dt);

	void UpdatePhysics3D(float dt);
	void Integrate3D(float dt);
	std::vector<CollisionPair3D> BroadPhase3D(float dt);
	void NarrowPhase3D(std::vector<CollisionPair3D>& pairs, float dt);

	void SolveContinuous(Physics3D* body, float dt);

    // Dynamic BVH Core Functions
    int AllocateNode();
    void FreeNode(int nodeIndex);
    AABB ComputeAABB(Physics3D* body);
    void InsertLeaf(int leafNodeIndex);
    void RemoveLeaf(int leafNodeIndex);
    void QueryTree(int nodeIndex, Physics3D* queryBody, const AABB& queryAABB, std::vector<CollisionPair3D>& outPairs);

    // Dynamic BVH Core Functions for 2D
    int AllocateNode2D();
    void FreeNode2D(int nodeIndex);
    AABB2D ComputeAABB2D(Physics2D* body);
    void InsertLeaf2D(int leafNodeIndex);
    void RemoveLeaf2D(int leafNodeIndex);
    void QueryTree2D(int nodeIndex, Physics2D* queryBody, const AABB2D& queryAABB, std::vector<CollisionPair2D>& outPairs);

	std::vector<Physics2D*> bodies2D;
	std::vector<Physics3D*> bodies3D;

	std::map<std::pair<ObjectType, ObjectType>, CollisionMode> collisionMaskMap;

	static constexpr int   MAX_CCD_ITERATIONS = 4;
	static constexpr float SKIN_WIDTH         = 0.005f;

    // Tree State
    int rootNodeIndex = -1;
    int freeListIndex = 0;
    std::vector<BVHNode> bvhNodes;

    // Map to quickly find a body's leaf node index
    std::unordered_map<Physics3D*, int> bodyNodeMap;

    // Tree State for 2D
    int rootNodeIndex2D = -1;
    int freeListIndex2D = 0;
    std::vector<BVHNode2D> bvhNodes2D;
    std::unordered_map<Physics2D*, int> bodyNodeMap2D;

    const glm::vec2 FAT_BOUNDS_MARGIN_2D = { 0.2f, 0.2f };
    const glm::vec3 FAT_BOUNDS_MARGIN = { 0.2f, 0.2f, 0.2f };
};
