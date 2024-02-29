//Author: DOYEONG LEE
//Project: CubeEngine
//File: PocketBallDemo.hpp
#pragma once
#include "GameState.hpp"
#include "PocketBallDemo/PocketBallSystem.hpp"

#include <vector>

class PocketBallDemo : public GameState
{
public:
	PocketBallDemo() = default;
	~PocketBallDemo() override {}

	void Init() override;
	void Update(float dt) override;
#ifdef _DEBUG
	void ImGuiDraw(float dt) override;
#endif
	void Restart() override;
	void End() override;

private:
	PocketBallSystem* pocketBallSystem = nullptr;
	int ballAmount = 0;
};