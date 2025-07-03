//Author: DOYEONG LEE
//Project: CubeEngine
//File: ThreadManager.cpp
#include "Engine.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"

void ThreadManager::Start()
{
	running = true;
	gameUpdateThread = std::thread(&ThreadManager::GameUpdateLoop, this);
	sdlEventThread = std::thread(&ThreadManager::SDLEventLoop, this);
}

void ThreadManager::Stop()
{
	running = false;
	gameUpdateCV.notify_one();
	if (gameUpdateThread.joinable()) gameUpdateThread.join();
	if (sdlEventThread.joinable()) sdlEventThread.join();
}

void ThreadManager::QueueGameUpdate(const std::function<void(float)>& updateFunc, float dt)
{
	{
		std::lock_guard<std::mutex> lock(gameUpdateMutex);
		gameUpdateQueue.push({ updateFunc, dt });
	}
	gameUpdateCV.notify_one();
}

void ThreadManager::WaitForGameUpdates()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(gameUpdateMutex);
		if (gameUpdateQueue.empty())
		{
			break;
		}
		lock.unlock();
		std::this_thread::yield();
	}
}

void ThreadManager::GameUpdateLoop()
{
	while (running)
	{
		std::unique_lock<std::mutex> lock(gameUpdateMutex);
		gameUpdateCV.wait(lock, [this] { return !running || !gameUpdateQueue.empty(); });

		while (!gameUpdateQueue.empty()) {
			auto [updateFunc, dt] = gameUpdateQueue.front();
			gameUpdateQueue.pop();
			lock.unlock();

			updateFunc(dt);

			lock.lock();
		}
	}
}

void ThreadManager::SDLEventLoop()
{
	while (running)
	{
		std::this_thread::yield();
	}
}

void ThreadManager::ProcessEvents()
{
	Engine::GetInputManager().ResetWheelMotion();
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL3_ProcessEvent(&event);

		if (ImGui::IsAnyItemActive() == false)
		{
			Engine::GetInputManager().InputPollEvent(event);
		}

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			Engine::GetGameStateManager().SetGameState(State::UNLOAD);
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			Engine::Instance().ResetDeltaTime();
			if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::DX)
				dynamic_cast<DXRenderManager*>(Engine::GetRenderManager())->SetResize(event.window.data1, event.window.data2);
			SDL_FALLTHROUGH;
		case SDL_EVENT_WINDOW_MOVED:
		case SDL_EVENT_WINDOW_MINIMIZED:
		case SDL_EVENT_WINDOW_MAXIMIZED:
		case SDL_EVENT_WINDOW_RESTORED:
			Engine::Instance().ResetDeltaTime();
			break;
		default:
			break;
		}

		if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
		{
			Engine::GetWindow().UpdateWindowGL(event);
		}
	}
}