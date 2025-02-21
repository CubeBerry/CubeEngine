//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: VerticesDemo.hpp
#pragma once
#include "GameState.hpp"
#include "Object.hpp"

#include <vector>

class VerticesDemo : public GameState
{
public:
	VerticesDemo() = default;
	~VerticesDemo() override {}

	void Init() override;
	void Update(float dt) override;
	void ImGuiDraw(float dt) override;
	void Restart() override;
	void End() override;
};