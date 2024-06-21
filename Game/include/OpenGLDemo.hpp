//Author: JEYOON YU
//Project: CubeEngine
//File: OpenGLDemo.hpp
#pragma once
#include "GameState.hpp"
#include "Object.hpp"

#include <vector>

class OpenGLDemo : public GameState
{
public:
	OpenGLDemo() = default;
	~OpenGLDemo() override {}

	void Init() override;
	void Update(float dt) override;
#ifdef _DEBUG
	void ImGuiDraw(float dt) override;
#endif
	void Restart() override;
	void End() override;
private:
};