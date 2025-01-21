//Author: JEYOON YU
//Project: CubeEngine
//File: PBR.hpp
#pragma once
#include "GameState.hpp"
#include "RenderManager.hpp"
#include <array>

class PBR : public GameState
{
public:
	PBR() = default;
	~PBR() override {}

	void Init() override;
	void Update(float dt) override;
	void ImGuiDraw(float dt) override;
	void Restart() override;
	void End() override;
private:
	float cNear{ 0.05f }, cFar{ 45.f };
	float cFov{ 22.5f };
	bool isDrawNormals{ false };
};
