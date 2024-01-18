//Author: DOYEONG LEE
//Project: CubeEngine
//File: Window.hpp
#pragma once
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "SDL2/SDL_syswm.h"

#include "glm/vec2.hpp"

enum class WindowMode
{
	NORMAL,
	BORADLESS,
	NONE
};

class VKInit;

class Window
{
public:
	Window() = default;
	void Init(const char* title, int width, int height, bool fullscreen, WindowMode mode);

	SDL_Window* GetWindow() { return window; };
	bool GetQuit() { return isQuit; };
	bool GetMinimized() { return isMinimized; };

	glm::vec2 GetWindowSize() { return wSize; }
private:
	SDL_Window* window;
	SDL_Event e;

	glm::vec2 wSize = { 0,0 };
	bool isMinimized{ false };
	bool isQuit{ false };
};