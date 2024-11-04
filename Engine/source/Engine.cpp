//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: Engine.cpp
#include "Engine.hpp"

void Engine::Init(const char* title, int windowWidth, int windowHeight, bool fullScreen, WindowMode mode)
{
	std::cout << "0. OpenGL    1. Vulkan";
	std::cout << std::endl << std::endl;
	std::cout << "Select API number: ";
	int number{ 0 };
	std::cin >> number;
	std::cout << std::endl;

	//Init Window -> Init VKInit -> Init SwapChain -> Init VKRenderManager
	//window = new Window();
	if (number == 0)
	{
		window.Init(GraphicsMode::GL, title, windowWidth, windowHeight, fullScreen, mode);
		renderManager = new GLRenderManager;
		dynamic_cast<GLRenderManager*>(renderManager)->Initialize(
			window.GetWindow(), window.GetContext()
		);
		//renderManager->Initialize(window.GetWindow(), window.GetContext());
	}
	else
	{
		window.Init(GraphicsMode::VK, title, windowWidth, windowHeight, fullScreen, mode);
		renderManager = new VKRenderManager;
		dynamic_cast<VKRenderManager*>(renderManager)->Initialize(window.GetWindow());
		//renderManager->Initialize(window.GetWindow());
	}
	timer.Init();

	cameraManager.Init({ windowWidth ,windowHeight }, CameraType::TwoDimension, 1.f);
	soundManager.Initialize(8);

	threadManager.Start();
}

void Engine::Update()
{
	SDL_Event event;
	while (gameStateManger.GetGameState() != State::SHUTDOWN)
	{
		timer.Update();
		deltaTime = timer.GetDeltaTime();
		if (timer.GetFrameRate() == FrameRate::UNLIMIT || deltaTime >= timer.GetFramePerTime())
		{
			Uint32 winFlag = SDL_GetWindowFlags(window.GetWindow());
			threadManager.ProcessSDLEvents();
			SDL_PollEvent(&event);

			timer.ResetLastTimeStamp();
			frameCount++;
			if (frameCount >= static_cast<int>(timer.GetFrameRate()))
			{
				if (!(winFlag & SDL_WINDOW_MINIMIZED))
				{
					int averageFrameRate = static_cast<int>(frameCount / timer.GetFrameRateCalculateTime());
					windowTitleWithFrameCount = " (fps: " + std::to_string(averageFrameRate) + ")";
					window.SetSubWindowTitle(windowTitleWithFrameCount);
				}
				timer.ResetFPSCalculateTime();
				frameCount = 0;

			}//fps
			gameStateManger.Update(deltaTime);
		}
	}
}

void Engine::End()
{
	if (renderManager->GetGraphicsMode() == GraphicsMode::GL)
		delete dynamic_cast<GLRenderManager*>(renderManager);
	else
		delete dynamic_cast<VKRenderManager*>(renderManager);
	threadManager.Stop();
}

void Engine::SetFPS(FrameRate fps)
{
	timer.Init(fps);
}

void Engine::ResetDeltaTime()
{
	deltaTime = 0.f;
}
