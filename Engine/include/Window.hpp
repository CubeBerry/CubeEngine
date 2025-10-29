//Author: DOYEONG LEE
//Project: CubeEngine
//File: Window.hpp
#pragma once
#include "SDL3/SDL.h"

#include "glm/vec2.hpp"
#include <memory>
#include <string>

enum class GraphicsMode
{
	GL, VK, DX
};

enum class WindowMode
{
	NORMAL,
	BORADLESS,
	NONE
};

class Window
{
public:
	Window() = default;
	~Window();
	void Init(GraphicsMode gMode, const char* title, int width, int height, bool fullscreen, WindowMode wMode);
	void InitWindowGL(WindowMode wMode, const char* title, int flags);
	void UpdateWindowGL(SDL_Event& event);

	SDL_Window* GetWindow() { return window.get(); };
	SDL_GLContext GetContext() { return context; };
	//bool GetMinimized() { return isMinimized; };

	glm::vec2 GetWindowSize() { return wSize; }
	std::string GetWindowTitle() { return titleName; }

	void SetWindowSize(glm::vec2 wSize);
	void SetMainWindowTitle(std::string title);
	void SetSubWindowTitle(std::string subTitle);
private:
	//SDL_Window* window;
	struct SDLWindowDestroyer
	{
		void operator()(SDL_Window* w) const
		{
			SDL_DestroyWindow(w);
		}
	};
	std::unique_ptr<SDL_Window, SDLWindowDestroyer> window;
	SDL_GLContext context;

	std::string titleName = "";
	glm::vec2 wSize = { 0,0 };
	//bool isMinimized{ false };
};
