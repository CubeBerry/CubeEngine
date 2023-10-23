#include "Window.hpp"
#include "VKInit.hpp"

#include <iostream>

void Window::Init(const char* title, int width, int height, bool fullscreen, WindowMode mode)
{
	isQuit = false;

	//Init Window -> Init VKInit -> Init SwapChain -> Init VKRenderManager
	int flags = 0;
	wSize.x = (float)width;
	wSize.y = (float)height;

	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
	}

	else
	{
		switch (mode)
		{
		case WindowMode::NORMAL:
			window = SDL_CreateWindow(
				title,
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				(int)wSize.x,
				(int)wSize.y,
				flags | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
			break;
		case WindowMode::BORADLESS:
			window = SDL_CreateWindow(
				title,
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				(int)wSize.x,
				(int)wSize.y,
				flags | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
			break;
		default:
			break;
		};
		std::cout << "Create Window Successful" << std::endl;
	}
}