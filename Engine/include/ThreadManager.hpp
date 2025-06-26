//Author: DOYEONG LEE
//Project: CubeEngine
//File: ThreadManager.hpp
#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <SDL3/SDL.h>

class ThreadManager {
public:
    ThreadManager() : running(false) {}
    ~ThreadManager() { Stop(); }

    void Start();
    void Stop();

    void QueueGameUpdate(const std::function<void(float)>& updateFunc, float dt);
    void WaitForGameUpdates();

    void ProcessEvents();

private:
    void GameUpdateLoop();
    void SDLEventLoop();

    std::thread gameUpdateThread;
    std::thread sdlEventThread;

    std::mutex gameUpdateMutex;
    std::condition_variable gameUpdateCV;
    std::queue<std::pair<std::function<void(float)>, float>> gameUpdateQueue;

    std::mutex sdlEventMutex;
    std::queue<SDL_Event> sdlEventQueue;

    std::atomic<bool> running;
};