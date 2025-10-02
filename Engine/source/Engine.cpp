//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: Engine.cpp

#include "Engine.hpp"

void Engine::Init(const char* title, int windowWidth, int windowHeight, bool fullScreen, WindowMode mode)
{
	std::cout << "0. OpenGL    1. Vulkan	2. DirectX 12";
	std::cout << "\n\n";
	std::cout << "Select API number: ";
	int number{ 0 };
	std::cin >> number;
	std::cout << '\n';

	if (number == 0)
	{
		window.Init(GraphicsMode::GL, title, windowWidth, windowHeight, fullScreen, mode);
		renderManager = new GLRenderManager;
		dynamic_cast<GLRenderManager*>(renderManager)->Initialize(
			window.GetWindow(), window.GetContext()
		);
	}
	else if (number == 1)
	{
		window.Init(GraphicsMode::VK, title, windowWidth, windowHeight, fullScreen, mode);
		renderManager = new VKRenderManager;
		dynamic_cast<VKRenderManager*>(renderManager)->Initialize(window.GetWindow());
	}
	else
	{
		window.Init(GraphicsMode::DX, title, windowWidth, windowHeight, fullScreen, mode);
		renderManager = new DXRenderManager;
		dynamic_cast<DXRenderManager*>(renderManager)->Initialize(window.GetWindow());
	}
	timer.Init();

	cameraManager.Init({ windowWidth ,windowHeight }, CameraType::TwoDimension, 1.f);
	soundManager.Initialize(8);

	threadManager.Start();
}

void Engine::Update()
{
	while (gameStateManger.GetGameState() != State::SHUTDOWN)
	{
		timer.Update();
		deltaTime = timer.GetDeltaTime();

		if (deltaTime > 0.1f)
		{
			deltaTime = 0.1f;
		}
		if (timer.GetFrameRate() == FrameRate::UNLIMIT || deltaTime >= timer.GetFramePerTime())
		{
			Uint64 winFlag = SDL_GetWindowFlags(window.GetWindow());
			threadManager.ProcessEvents();
			timer.ResetLastTimeStamp();
			frameCount++;
			if (frameCount >= static_cast<int>(timer.GetFrameRate()))
			{
				timer.AddFrameHistory(frameCount / timer.GetFrameRateCalculateTime());
				timer.ResetFPSCalculateTime();
				frameCount = 0;

			}//fps

			gameStateManger.Update(deltaTime);
		}
	}
}

void Engine::End()
{
	switch (renderManager->GetGraphicsMode())
	{
	case GraphicsMode::GL:
		delete dynamic_cast<GLRenderManager*>(renderManager);
		break;
	case GraphicsMode::VK:
		delete dynamic_cast<VKRenderManager*>(renderManager);
		break;
	case GraphicsMode::DX:
		delete dynamic_cast<DXRenderManager*>(renderManager);
		break;
	}

	threadManager.Stop();
}

void Engine::SetFPS(FrameRate fps)
{
	ResetDeltaTime();
	timer.Init(fps);
	frameCount = 0;
}

void Engine::ResetDeltaTime()
{
	timer.Reset();
	deltaTime = 0.f;
}