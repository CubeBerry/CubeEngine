//Author: DOYEONG LEE
//Project: CubeEngine
//File: 3DPhysicsDemo.hpp
#pragma once
#include "GameState.hpp"
#include "Object.hpp"

#include <vector>

class PhysicsDemo : public GameState
{
public:
	PhysicsDemo() = default;
	~PhysicsDemo() override {}

	void Init() override;
	void Update(float dt) override;
#ifdef _DEBUG
	void ImGuiDraw(float dt) override;
#endif
	void Restart() override;
	void End() override;
};