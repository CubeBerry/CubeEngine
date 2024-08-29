//Author: JEYOON YU
//Project: CubeEngine
//File: GLImGuiManager.hpp
#pragma once
#include <SDL.h>

class GLImGuiManager
{
public:
	GLImGuiManager(SDL_Window* window_, SDL_GLContext context_);
	~GLImGuiManager();

	void Initialize(SDL_Window* window_, SDL_GLContext context_);
	//void FeedEvent(const SDL_Event& event_);
	void Begin();
	void End();
	void Shutdown();
private:
};
