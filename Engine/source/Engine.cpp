#include "Engine.hpp"

#include"levels/ShaderDemo.hpp"
#include"levels/VerticesDemo.hpp"
#include <iostream>

void Engine::Init()
{
	window = new Window();
	window->Init("Vulkan Demo", 640, 480, false, WindowMode::NORMAL);
	timer.Init(FPS_144);

	vkRenderManager = new VKRenderManager(window->GetWindow());
	gameStateManger = new GameStateManager();
	inputManager = new InputManager;

	gameStateManger->AddLevel(new ShaderDemo);
	gameStateManger->AddLevel(new VerticesDemo);

	gameStateManger->LevelInit();
}

void Engine::Update()
{
	SDL_Event event;
	while (isRunning)
	{
		timer.Update();
		deltaTime = timer.GetDeltaTime();

		if (deltaTime > 1.f / static_cast<float>(timer.GetFrameRate()))
		{
			SDL_PollEvent(&event);
			switch (event.type)
			{
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_WINDOWEVENT:
				//window->Update(&event);
				break;
			default:
				break;
			}
			inputManager->InputPollEvent(event);
			if (inputManager->IsKeyPressedOnce(KEYBOARDKEYS::ESCAPE))
			{
				SDL_Event quitEvent;
				quitEvent.type = SDL_QUIT;
				SDL_PushEvent(&quitEvent);
			}
			if (inputManager->IsKeyPressedOnce(KEYBOARDKEYS::R))
			{
				gameStateManger->RestartLevel();
			}

			gameStateManger->Draw(deltaTime);
			gameStateManger->Update(deltaTime);

			timer.ResetLastTimeStamp();
			frameCount++;
			if (frameCount >= timer.GetFrameRate())
			{
				int averageFrameRate = static_cast<int>(frameCount / timer.GetFrameRateCalculateTime());
				//windowTitleWithFrameCount = " (fps: " + std::to_string(averageFrameRate) + ")";
				//window.SetWindowTitle(windowTitleWithFrameCount.c_str());
				std::cout << averageFrameRate << std::endl;
				timer.ResetFPSCalculateTime();
				frameCount = 0;
			}//fps
		}

	}
}

void Engine::End()
{
	gameStateManger->End();

	delete inputManager;
	delete vkRenderManager;
	delete gameStateManger;
	delete window;
}
