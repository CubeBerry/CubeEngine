//Author: DOYEONG LEE
//Project: CubeEngine
//File: Engine.hpp
#pragma once
#include "Timer.hpp"
#include "Window.hpp"
#include "GameStateManager.hpp"
#include "VKInit.hpp"
#include "VKRenderManager.hpp"
#include "InputManager.hpp"
#include "ObjectManager.hpp"
#include "CameraManager.hpp"
#include "SoundManager.hpp"
#include "SpriteManager.hpp"

class Engine
{
public:
	Engine() {};
	~Engine() = default;

	void Init(const char* title, int windowWidth, int windowHeight, bool fullScreen, WindowMode mode);
	void Update();
	void End();

	void SetFPS(FrameRate fps);

	static Engine& Instance() { static Engine instance; return instance; }
	static Window* GetWindow() { return Instance().window; }
	static VKInit* GetVKInit() { return Instance().vkInit; }
	static VKRenderManager* GetVKRenderManager() { return Instance().vkRenderManager; }
	static GameStateManager* GetGameStateManager() { return Instance().gameStateManger; }
	static InputManager* GetInputManager() { return Instance().inputManager; }
	static ObjectManager* GetObjectManager() { return Instance().objectManager; }
	static CameraManager* GetCameraManager() { return Instance().cameraManager; }
	static SoundManager* GetSoundManager() { return Instance().soundManager; }
	static SpriteManager* GetSpriteManager() { return Instance().spriteManager; }
private:

	bool isRunning = true;

	Timer timer;
	std::chrono::system_clock::time_point lastTick;
	std::chrono::system_clock::time_point fpsCalcTime;

	float deltaTime = 0.f;
	int frameCount = 0;
	std::string windowTitleWithFrameCount;

	Window* window = nullptr;
	VKInit* vkInit = nullptr;
	VKRenderManager* vkRenderManager = nullptr;
	GameStateManager* gameStateManger = nullptr;
	InputManager* inputManager = nullptr;
	ObjectManager* objectManager = nullptr;
	CameraManager* cameraManager = nullptr;
	SoundManager* soundManager = nullptr;
	SpriteManager* spriteManager = nullptr;
};