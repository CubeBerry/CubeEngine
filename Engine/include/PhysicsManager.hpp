//Author: DOYEONG LEE
//Project: CubeEngine
//File: PhysicsManager.hpp
#pragma once

#include <vector>
#include "BasicComponents/Physics2D.hpp"
#include "BasicComponents/Physics3D.hpp"

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

private:
	void UpdatePhysics2D(float dt);
	std::vector<CollisionPair2D> BroadPhase2D();
	void ApplyMovement2D(float dt);
	void NarrowPhase2D(std::vector<CollisionPair2D>& pairs, float dt);

	void UpdatePhysics3D(float dt);
	void Integrate3D(float dt);
	std::vector<CollisionPair3D> BroadPhase3D(float dt);
	void NarrowPhase3D(std::vector<CollisionPair3D>& pairs, float dt);

	void SolveDiscrete_BoxBox    (Physics3D* a, Physics3D* b);
	void SolveDiscrete_SphereSphere(Physics3D* a, Physics3D* b);
	void SolveDiscrete_BoxSphere  (Physics3D* box, Physics3D* sphere);

	void SolveContinuous(Physics3D* body, float dt);

	std::vector<Physics2D*> bodies2D;
	std::vector<Physics3D*> bodies3D;

	static constexpr int   MAX_CCD_ITERATIONS = 4;
	static constexpr float SKIN_WIDTH         = 0.005f;
};
