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
#ifdef _DEBUG
			window.GetWindow(), window.GetContext()
#endif
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

	//soundManager = new SoundManager;
	soundManager.Initialize(8);

	//spriteManager = new SpriteManager;
	//particleManager = new ParticleManager();
}

void Engine::Update()
{
	SDL_Event event;
	while (gameStateManger.GetGameState() != State::SHUTDOWN)
	{
		timer.Update();
		deltaTime = timer.GetDeltaTime();
		if (deltaTime > 1.f / static_cast<float>(timer.GetFrameRate()))
		{
			SDL_PollEvent(&event);
#ifdef _DEBUG
			//ImGui
			ImGui_ImplSDL2_ProcessEvent(&event);
#endif
			switch (event.type)
			{
			case SDL_QUIT:
				gameStateManger.SetGameState(State::UNLOAD);
				break;
			//case SDL_WINDOWEVENT:
			//	if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || event.window.event == SDL_WINDOWEVENT_MOVED ||
			//		event.window.event == SDL_WINDOWEVENT_MINIMIZED)
			//	{
			//		deltaTime = 0.f;
			//		std::cout << deltaTime << ", Window" << std::endl;
			//	}
			//	break;
			default:
				break;
			}

			timer.ResetLastTimeStamp();
			frameCount++;
			if (frameCount >= static_cast<int>(timer.GetFrameRate()))
			{
				int averageFrameRate = static_cast<int>(frameCount / timer.GetFrameRateCalculateTime());
				//windowTitleWithFrameCount = " (fps: " + std::to_string(averageFrameRate) + ")";
				//window.SetWindowTitle(windowTitleWithFrameCount.c_str());
				std::cout << averageFrameRate << std::endl;
				timer.ResetFPSCalculateTime();
				frameCount = 0;
			}//fps

			inputManager.InputPollEvent(event);
			gameStateManger.Update(deltaTime);
			if (renderManager->GetGraphicsMode() == GraphicsMode::GL)
				window.UpdateWindowGL(event);
		}
	}
}

void Engine::End()
{
	if (renderManager->GetGraphicsMode() == GraphicsMode::GL)
		delete dynamic_cast<GLRenderManager*>(renderManager);
	else
		delete dynamic_cast<VKRenderManager*>(renderManager);
	//delete gameStateManger;
	//delete cameraManager;
	//delete inputManager;
	//delete particleManager;
	//delete objectManager;
	//delete spriteManager;
	//delete vkRenderManager;
	//delete soundManager;
	//delete window;
	//delete vkInit;
}

void Engine::SetFPS(FrameRate fps)
{
	timer.Init(fps);
}
