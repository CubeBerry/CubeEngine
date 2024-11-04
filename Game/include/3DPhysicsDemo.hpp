//Author: DOYEONG LEE
//Project: CubeEngine
//File: 3DPhysicsDemo.hpp
#pragma once
#include "GameState.hpp"
#include "Object.hpp"

#include <vector>

#include "Material.hpp"

class PhysicsDemo : public GameState
{
public:
	PhysicsDemo() = default;
	~PhysicsDemo() override {}

	void Init() override;
	void Update(float dt) override;
	void ImGuiDraw(float dt) override;
	void Restart() override;
	void End() override;
private:
	ThreeDimension::VertexLightingUniform l;
};
