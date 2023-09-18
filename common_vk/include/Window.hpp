#pragma once
#include <glm/vec2.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_syswm.h>

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
	void Update();

	SDL_Window* GetWindow() { return window; };
	bool GetQuit() { return isQuit; };
	bool GetMinimized() { return isMinimized; };
private:
	SDL_Window* window;
	SDL_Event e;

	glm::vec2 wSize = { 0,0 };
	bool isMinimized{ false };
	bool isQuit{ false };
};