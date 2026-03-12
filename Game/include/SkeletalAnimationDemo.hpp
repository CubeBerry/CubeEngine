//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimationDemo.hpp
#pragma once
#include "GameState.hpp"
#include "RenderManager.hpp"
#include <array>

class SkeletalAnimationDemo : public GameState
{
public:
	SkeletalAnimationDemo() = default;
	~SkeletalAnimationDemo() override {}

	void Init() override;
	void Update(float dt) override;
	void ImGuiDraw(float dt) override;
	void Restart() override;
	void End() override;
private:
	float cNear{ 0.001f }, cFar{ 1000.f };
	float cFov{ 22.5f };

	float time{ 0 };
};
