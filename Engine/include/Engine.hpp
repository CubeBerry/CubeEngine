//Author: DOYEONG LEE
//Project: CubeEngine
//File: Engine.hpp
#pragma once
#include "Timer.hpp"
#include "Window.hpp"
#include "RenderManager.hpp"
#include "GameStateManager.hpp"
#include "InputManager.hpp"
#include "ObjectManager.hpp"
#include "CameraManager.hpp"
#include "SoundManager.hpp"
#include "SpriteManager.hpp"
#include "JobSystem.hpp"
#include "Logger.hpp"
#include "Particle/ParticleManager.hpp"
#include "PhysicsManager.hpp"

class Engine
{
public:
	Engine() = default;
	~Engine() = default;

	static Engine& Instance() { static Engine instance; return instance; }
	static Window& GetWindow() { return Instance().window; }
	static RenderManager* GetRenderManager() { return Instance().renderManager; }
	static GameStateManager& GetGameStateManager() { return Instance().gameStateManger; }
	static InputManager& GetInputManager() { return Instance().inputManager; }
	static ObjectManager& GetObjectManager() { return Instance().objectManager; }
	static CameraManager& GetCameraManager() { return Instance().cameraManager; }
	static SoundManager& GetSoundManager() { return Instance().soundManager; }
	static SpriteManager& GetSpriteManager() { return *Instance().spriteManager; }
	static ParticleManager& GetParticleManager() { return Instance().particleManager; }
	static PhysicsManager& GetPhysicsManager() { return Instance().physicsManager; }
	static Timer& GetTimer() { return Instance().timer; }
	static Logger& GetLogger() { return *Instance().logger; }
	static JobSystem& GetJobSystem() { return Instance().jobSystem; }
	static const InputSnapshot& GetInputSnapshot() { return Instance().inputSnapshot; }

	void Init(const char* title, int windowWidth, int windowHeight, bool fullScreen, WindowMode mode);
	void Update();
	void End();

	void SetFPS(FrameRate fps);
	FrameRate GetFPS() { return timer.GetFrameRate(); }

	void ResetDeltaTime();
private:
	Timer timer;

	float deltaTime = 0.f;
	int frameCount = 0;
	std::string windowTitleWithFrameCount;

	Window window;
	RenderManager* renderManager;
	GameStateManager gameStateManger;
	InputManager inputManager;
	ObjectManager objectManager;
	CameraManager cameraManager;
	SoundManager soundManager;
	SpriteManager* spriteManager;
	ParticleManager particleManager;
	PhysicsManager physicsManager;
	JobSystem jobSystem;
	InputSnapshot inputSnapshot;
	Logger* logger;
};