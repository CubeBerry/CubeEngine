#pragma once
#include "GameState.hpp"
#include "Window.hpp"
#include "VKRenderManager.hpp"

class VerticesDemo : public GameState
{
public:
	VerticesDemo() = default;
	~VerticesDemo() {};

	void Init() override;
	void Update(float dt) override;
	void Draw(float dt) override;
	void Restart() override;
	void End() override;
private:
	Window* window = nullptr;
	VKRenderManager* renderManager = nullptr;
};