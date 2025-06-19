//Author: DOYEONG LEE
//Project: CubeEngine
//File: ThreadManager.cpp
#include "Engine.hpp"

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
	sdlEventCV.notify_one();
	if (gameUpdateThread.joinable()) gameUpdateThread.join();
	if (sdlEventThread.joinable()) sdlEventThread.join();
}

void ThreadManager::QueueGameUpdate(const std::function<void(float)>& updateFunc, float dt)
{
	{
		std::lock_guard<std::mutex> lock(gameUpdateMutex);
		gameUpdateQueue.push(std::make_pair(updateFunc, dt));
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
			auto updatePair = gameUpdateQueue.front();
			gameUpdateQueue.pop();
			lock.unlock();

			// Execute game update function
			updatePair.first(updatePair.second);

			lock.lock();
		}
	}
}

void ThreadManager::SDLEventLoop()
{
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			QueueSDLEvent(event);
		}
		std::this_thread::yield();
	}
}

void ThreadManager::QueueSDLEvent(const SDL_Event& event)
{
	{
		std::lock_guard<std::mutex> lock(sdlEventMutex);
		sdlEventQueue.push(event);

		//if (event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
		//	event.type == SDL_EVENT_MOUSE_BUTTON_UP || event.type == SDL_EVENT_MOUSE_WHEEL ||
		//	event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP ||
		//	event.type == SDL_EVENT_TEXT_INPUT || event.type == SDL_EVENT_QUIT)
		//{
			std::lock_guard<std::mutex> mainLock(mainThreadEventMutex);
			mainThreadEventQueue.push(event);
		//}
	}
	sdlEventCV.notify_one();
}

void ThreadManager::ProcessSDLEvents()
{
	std::queue<SDL_Event> localEventQueue;
	{
		std::lock_guard<std::mutex> lock(sdlEventMutex);
		std::swap(localEventQueue, sdlEventQueue);
	}

	while (!localEventQueue.empty()) {
		SDL_Event& event = localEventQueue.front();
		if(ImGui::IsAnyItemActive() == false)
		{
			Engine::GetInputManager().InputPollEvent(event);
		}
		localEventQueue.pop();
	}

	ProcessSDLEventsMainThread();
}

void ThreadManager::ProcessSDLEventsMainThread()
{
	std::queue<SDL_Event> localMainThreadEventQueue;
	{
		std::lock_guard<std::mutex> lock(mainThreadEventMutex);
		std::swap(localMainThreadEventQueue, mainThreadEventQueue);
	}

	while (!localMainThreadEventQueue.empty())
	{
		SDL_Event& event = localMainThreadEventQueue.front();
		ImGui_ImplSDL3_ProcessEvent(&event);

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			Engine::GetGameStateManager().SetGameState(State::UNLOAD);
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			// Let DX12 know window is resized
			if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::DX)
				dynamic_cast<DXRenderManager*>(Engine::GetRenderManager())->OnResize(event.window.data1, event.window.data2);
			SDL_FALLTHROUGH;
		case SDL_EVENT_WINDOW_MOVED:
		case SDL_EVENT_WINDOW_MINIMIZED:
		case SDL_EVENT_WINDOW_MAXIMIZED:
			Engine::Instance().ResetDeltaTime();
			break;
		default:
			break;
		}
		localMainThreadEventQueue.pop();
		if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
			Engine::GetWindow().UpdateWindowGL(event);
	}
}
