//Author: DOYEONG LEE
//Project: CubeEngine
//File: Window.cpp
#include "Window.hpp"
#include "glew/glew.h"
#include <Windows.h>

#include <iostream>

// Debugging
void APIENTRY OpenGLDebugMessageCallback(GLenum /*source*/, GLenum /*type*/, GLuint id, GLenum severity,
	GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		std::cerr << "OpenGL Debug Message [" << id << "]: " << message << '\n';
	}
}

void Window::Init(GraphicsMode gMode, const char* title, int width, int height, bool fullscreen, WindowMode wMode)
{
	int flags = 0;
	wSize.x = static_cast<float>(width);
	wSize.y = static_cast<float>(height);

	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
	}
	else
	{
		switch (gMode)
		{
		case GraphicsMode::GL:
			InitWindowGL(wMode, title, flags);
			break;
		case GraphicsMode::VK:
			switch (wMode)
			{
			case WindowMode::NORMAL:
				window = std::unique_ptr<SDL_Window, SDLWindowDestroyer>(SDL_CreateWindow(
					title,
					static_cast<int>(wSize.x),
					static_cast<int>(wSize.y),
					flags | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN));
				break;
			case WindowMode::BORADLESS:
				window = std::unique_ptr<SDL_Window, SDLWindowDestroyer>(SDL_CreateWindow(
					title,
					static_cast<int>(wSize.x),
					static_cast<int>(wSize.y),
					flags | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN));
				break;
			}
			break;
		case GraphicsMode::DX:
			switch (wMode)
			{
			case WindowMode::NORMAL:
				window = std::unique_ptr<SDL_Window, SDLWindowDestroyer>(SDL_CreateWindow(
					title,
					static_cast<int>(wSize.x),
					static_cast<int>(wSize.y),
					flags | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN));
				break;
			case WindowMode::BORADLESS:
				window = std::unique_ptr<SDL_Window, SDLWindowDestroyer>(SDL_CreateWindow(
					title,
					static_cast<int>(wSize.x),
					static_cast<int>(wSize.y),
					flags | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN));
				break;
			}
			break;
		}
		SetMainWindowTitle(title);
		std::cout << "Create Window Successful" << '\n';
	}
}

void Window::InitWindowGL(WindowMode wMode, const char* title, int flags)
{
	// Use Modern OpenGL Code Only
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	// Use OpenGL Version 4.6
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	// Use Double Buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
	//SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	// MultiSampling
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

	// Enable Debug
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	switch (wMode)
	{
	case WindowMode::NORMAL:
		window = std::unique_ptr<SDL_Window, SDLWindowDestroyer>(SDL_CreateWindow(
			title,
			static_cast<int>(wSize.x),
			static_cast<int>(wSize.y),
			flags | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL));
		break;
	case WindowMode::BORADLESS:
		window = std::unique_ptr<SDL_Window, SDLWindowDestroyer>(SDL_CreateWindow(
			title,
			static_cast<int>(wSize.x),
			static_cast<int>(wSize.y),
			flags | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL));
		break;
	}

	// Make Context
	context = SDL_GL_CreateContext(window.get());
	SDL_GL_MakeCurrent(window.get(), context);

	// Init GLEW
	glewExperimental = true;
	GLenum result = glewInit();
	if (result != GLEW_OK)
		std::cerr << "GLEW Init Failed: " << glewGetErrorString(result) << '\n';

	// Debugging
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(OpenGLDebugMessageCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	// VSYNC
	//constexpr int ADAPTIVE_VSYNC = -1;
	//constexpr int VSYNC = 1;
	//if (const auto result = SDL_GL_SetSwapInterval(ADAPTIVE_VSYNC); result != 0)
	//{
	//	SDL_GL_SetSwapInterval(VSYNC);
	//}

	// Anti-Aliasing
	//glEnable(GL_LINE_SMOOTH);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// MSAA Anti-Aliasing
	glEnable(GL_MULTISAMPLE);

	// Blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Window::UpdateWindowGL(SDL_Event& event)
{
	if (event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
	{
		glViewport(0, 0, event.window.data1, event.window.data2);
	}
	//SDL_GL_SwapWindow(window.get());
}

void Window::SetWindowSize(glm::vec2 size)
{
	wSize = size;
}

void Window::SetMainWindowTitle(std::string title)
{
	titleName = title;
	SDL_SetWindowTitle(window.get(), titleName.c_str());
}

void Window::SetSubWindowTitle(std::string subTitle)
{
	SDL_SetWindowTitle(window.get(), (titleName + subTitle).c_str());
}
