#pragma once
#include "GameState.hpp"

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
};