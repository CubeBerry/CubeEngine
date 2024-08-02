//Author: DOYEONG LEE
//Project: CubeEngine
//File: BeatEmUpDemo.cpp
#include "BeatEmUpDemo/BeatEmUpDemo.hpp"
#include "Engine.hpp"

#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"

#include "BeatEmUpDemo/BEUObject.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

void BeatEmUpDemo::Init()
{
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 45.f);
	Engine::GetCameraManager().SetFar(90.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,5.f, 45.f });
	Engine::GetCameraManager().UpdaetCameraDirectrion({ 0.f, -5.f });

	beatEmUpDemoSystem = new BeatEmUpDemoSystem();
	beatEmUpDemoSystem->Init();

	Engine::GetObjectManager().AddObject<BEUObject>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 4.f, 6.f,0.f }, "Player", beatEmUpDemoSystem);
}

void BeatEmUpDemo::Update(float dt)
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

	beatEmUpDemoSystem->Update(dt);
}

#ifdef _DEBUG
void BeatEmUpDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
	Engine::GetGameStateManager().StateChanger();
	Engine::GetSoundManager().MusicPlayerForImGui(0);
}
#endif

void BeatEmUpDemo::Restart()
{
	End();
	Init();
}

void BeatEmUpDemo::End()
{
	beatEmUpDemoSystem->End();
	delete beatEmUpDemoSystem;
	beatEmUpDemoSystem = nullptr;

	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}

