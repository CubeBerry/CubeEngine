#pragma once
#include "Timer.hpp"
#include "Window.hpp"
#include "GameStateManager.hpp"
#include "VKRenderManager.hpp"
#include "InputManager.hpp"

class Engine 
{
public:
	Engine() {};
	~Engine() = default;

	void Init();
	void Update();
	void End();


	static Engine& Instance() { static Engine instance; return instance; }
	static Window* GetWindow() { return Instance().window; }
	static VKRenderManager* GetVKRenderManager() { return Instance().vkRenderManager; }
	static GameStateManager* GetGameStateManager() { return Instance().gameStateManger; }
	static InputManager* GetInputManager() { return Instance().inputManager; }
private:

	bool isRunning = true;

	Timer timer;
	std::chrono::system_clock::time_point lastTick;
	std::chrono::system_clock::time_point fpsCalcTime;

	float deltaTime = 0.f;
	int frameCount = 0;
	std::string windowTitleWithFrameCount;

	Window* window = nullptr;
	VKRenderManager* vkRenderManager = nullptr;
	GameStateManager* gameStateManger = nullptr;
	InputManager* inputManager = nullptr;
};