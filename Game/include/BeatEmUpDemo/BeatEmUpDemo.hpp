//Author: DOYEONG LEE
//Project: CubeEngine
//File: BeatEmUpDemo.hpp
#pragma once
#include "GameState.hpp"
#include "BeatEmUpDemo/BeatEmUpDemoSystem.hpp"

#include <vector>

class BeatEmUpDemo : public GameState
{
public:
	BeatEmUpDemo() = default;
	~BeatEmUpDemo() override {}

	void Init() override;
	void Update(float dt) override;
	void ImGuiDraw(float dt) override;
	void Restart() override;
	void End() override;

private:
	BeatEmUpDemoSystem* beatEmUpDemoSystem = nullptr;
};