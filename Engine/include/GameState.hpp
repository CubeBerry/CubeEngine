//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: GameState.hpp
#pragma once

class GameState
{
public:
	virtual ~GameState() {}

	virtual void Init() = 0;
	virtual void Update(float dt) = 0;
#ifdef _DEBUG
	virtual void ImGuiDraw(float dt) = 0;
#endif
	virtual void Restart() = 0;
	virtual void End() = 0;
protected:
};