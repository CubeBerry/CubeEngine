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
	~PocketBallDemo() {};

	void Init() override;
	void Update(float dt) override;
#ifdef _DEBUG
	void ImGuiDraw(float dt) override;
#endif
	void Restart() override;
	void End() override;

private:
	void CollideObjects();

	PocketBallSystem* pocketBallSystem = nullptr;
	int ballAmount = 0;
};