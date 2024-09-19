//Author: DOYEONG LEE
//Project: CubeEngine
//File: ThreadManager.hpp
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <SDL.h>

class ThreadManager {
public:
    ThreadManager() : running(false) {}
    ~ThreadManager() { Stop(); }

    void Start();
    void Stop();

    void QueueGameUpdate(const std::function<void(float)>& updateFunc, float dt);
    void WaitForGameUpdates();

    void ProcessSDLEvents();
    void QueueSDLEvent(const SDL_Event& event);
private:
    void GameUpdateLoop();

    void SDLEventLoop();
    void ProcessSDLEventsMainThread();

    std::thread gameUpdateThread;
    std::thread sdlEventThread;

    std::mutex gameUpdateMutex;
    std::mutex sdlEventMutex;
    std::mutex mainThreadEventMutex;

    std::condition_variable gameUpdateCV;
    std::condition_variable sdlEventCV;

    std::queue<std::pair<std::function<void(float)>, float>> gameUpdateQueue;
    std::queue<SDL_Event> sdlEventQueue;
    std::queue<SDL_Event> mainThreadEventQueue;
    
    std::atomic<bool> running;
};