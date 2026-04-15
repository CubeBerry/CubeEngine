//Author: DOYEONG LEE
//Project: CubeEngine
//File: 3DPhysicsDemo.hpp
#pragma once
#include "GameState.hpp"
#include "Object.hpp"

#include <vector>

#include "Material.hpp"

enum class PhysicsMode
{
	TwoDimension,
	ThreeDimension
};

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

	void Init3D();
	void Init2D();

	void Spawn3D();
	void Spawn2D();

	void ClearScene();

private:
	ThreeDimension::PointLightUniform l;
	PhysicsMode mode = PhysicsMode::ThreeDimension;
};
