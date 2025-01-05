//Author: DOYEONG LEE
//Project: CubeEngine
//File: PlatformDemo.cpp
#include "PlatformDemo/PlatformDemo.hpp"
#include "Engine.hpp"

#include "PlatformDemo/PPlayer.hpp"
#include "PlatformDemo/PEnemy.hpp"
#include "PlatformDemo/PBullet.hpp"
#include "PlatformDemo/PEnemyBullet.hpp"

#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

void PlatformDemo::Init()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::TwoDimension);
	Engine::Instance().GetCameraManager().Init(Engine::Instance().GetWindow().GetWindowSize(), CameraType::TwoDimension, 1.f);

	platformDemoSystem = new PlatformDemoSystem();
	platformDemoSystem->Init();

	Engine::GetRenderManager()->LoadTexture("../Game/assets/PlatformDemo/train_editor.png", "train_editor", true);
	Engine::GetRenderManager()->LoadTexture("../Game/assets/PlatformDemo/building1.png", "building1", true);
	Engine::GetRenderManager()->LoadTexture("../Game/assets/PlatformDemo/building2.png", "building2", true);
	Engine::GetRenderManager()->LoadTexture("../Game/assets/PlatformDemo/building3.png", "building3", true);
	Engine::GetRenderManager()->LoadTexture("../Game/assets/PlatformDemo/rail.png", "rail", true);
	Engine::GetRenderManager()->LoadTexture("../Game/assets/PlatformDemo/TrainSide.png", "trainSide", true);

	platformDemoSystem->LoadLevelData("../Game/assets/PlatformDemo/Stage.txt");
	Engine::GetObjectManager().AddObject<PPlayer>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 64.f, 96.f,0.f }, "Player", platformDemoSystem);
	Engine::GetObjectManager().AddObject<PEnemy>(glm::vec3{ -64.f,196.f,0.f }, glm::vec3{ 64.f, 96.f,0.f }, "Enemy", EnemyType::NORMAL);
	Engine::GetObjectManager().AddObject<PEnemy>(glm::vec3{ 640.f,0.f,0.f }, glm::vec3{ 320.f, 320.f,0.f }, "Enemy", EnemyType::AIRSHIP);
	platformDemoSystem->InitHealthBar();
}

void PlatformDemo::Update(float dt)
{
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::NUMBER_1))
	{
		Engine::GetGameStateManager().ChangeLevel(GameLevel::POCKETBALL);
	}
	else if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::NUMBER_2))
	{
		Engine::GetGameStateManager().ChangeLevel(GameLevel::PLATFORMDEMO);
	}
	else if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::R))
	{
		Engine::GetGameStateManager().SetGameState(State::RESTART);
	}

	platformDemoSystem->Update(dt);
}

void PlatformDemo::ImGuiDraw(float /*dt*/)
{
	Engine::GetGameStateManager().StateChanger();
	Engine::GetSoundManager().MusicPlayerForImGui(0);
	//platformDemoSystem->UpdateMapEditorImGui();
}

void PlatformDemo::Restart()
{
	End();
	Init();
}

void PlatformDemo::End()
{
	platformDemoSystem->End();
	delete platformDemoSystem;
	platformDemoSystem = nullptr;

	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}

