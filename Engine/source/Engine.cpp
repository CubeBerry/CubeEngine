#include "Engine.hpp"

void Engine::Init(const char* title, int windowWidth, int windowHeight, bool fullScreen, WindowMode mode)
{
	//Init Window -> Init VKInit -> Init SwapChain -> Init VKRenderManager
	window = new Window();
	window->Init(title, windowWidth, windowHeight, fullScreen, mode);
	timer.Init();

	vkRenderManager = new VKRenderManager(true);
	gameStateManger = new GameStateManager();
	inputManager = new InputManager;
	objectManager = new ObjectManager;
	cameraManager = new CameraManager;
	cameraManager->Init({ windowWidth ,windowHeight }, CameraType::ThreeDimension);
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
			//vkRenderManager->GetImGuiManager()->FeedEvent(event);
			switch (event.type)
			{
			case SDL_QUIT:
				isRunning = false;
				break;
			default:
				break;
			}

			inputManager->InputPollEvent(event);
			gameStateManger->Update(deltaTime);
			objectManager->Update(deltaTime);
			cameraManager->Update();
			if (!(SDL_GetWindowFlags(window->GetWindow()) & SDL_WINDOW_MINIMIZED))
				gameStateManger->Draw(deltaTime);

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
		}

	}
}

void Engine::End()
{
	delete cameraManager;
	delete inputManager;
	delete vkRenderManager;
	delete gameStateManger;
	delete objectManager;
	delete window;
}

void Engine::SetFPS(FrameRate fps)
{
	timer.Init(fps);
}
