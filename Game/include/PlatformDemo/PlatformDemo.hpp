//Author: DOYEONG LEE
//Project: CubeEngine
//File: PlatformDemo.hpp
#pragma once
#include "GameState.hpp"
#include "PlatformDemo/PlatformDemoSystem.hpp"

#include <vector>

class PlatformDemo : public GameState
{
public:
	PlatformDemo() = default;
	~PlatformDemo() override {}

	void Init() override;
	void Update(float dt) override;
	void ImGuiDraw(float dt) override;
	void Restart() override;
	void End() override;

private:
	PlatformDemoSystem* platformDemoSystem = nullptr;
};