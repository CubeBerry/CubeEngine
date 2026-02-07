//Author: JEYOON YU
//Project: CubeEngine
//File: MultipleLights.hpp
#pragma once
#include "GameState.hpp"
#include "Object.hpp"

#include <vector>

#include "Material.hpp"

class MultipleLights : public GameState
{
public:
	MultipleLights() = default;
	~MultipleLights() override {}

	void Init() override;
	void Update(float dt) override;
	void ImGuiDraw(float dt) override;
	void Restart() override;
	void End() override;
private:
	ThreeDimension::PointLightUniform l;
};
