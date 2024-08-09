//Author: DOYEONG LEE
//Project: CubeEngine
//File: BeatEmUpDemo.cpp
#include "BeatEmUpDemo/BeatEmUpDemo.hpp"
#include "Engine.hpp"

#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"

#include "BeatEmUpDemo/BEUPlayer.hpp"
#include "BeatEmUpDemo/BEUEnemy.hpp"
#include "BeatEmUpDemo/BEUAttackBox.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

void BeatEmUpDemo::Init()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::TwoDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 1.f);
	Engine::GetCameraManager().SetFar(91.f);
	Engine::GetCameraManager().SetBaseFov(45.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,10.f, 30.f });
	Engine::GetCameraManager().UpdaetCameraDirectrion({ 0.f, 10.f });

	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/road.png", "road");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/road1.png", "road1");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/hpbar.png", "hpbar");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/1.png", "1");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/2.png", "2");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/3.png", "3");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/4.png", "4");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/4_1.png", "4_1");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/4_2.png", "4_2");

	beatEmUpDemoSystem = new BeatEmUpDemoSystem();

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -25.f,-3.f,-16.5f }, glm::vec3{ 25.f, 21.5f,0.f }, "-1");
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("road");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,-3.f,-16.5f }, glm::vec3{ 25.f, 21.5f,0.f }, "1");
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("road");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 25.f,-3.f,-16.5f }, glm::vec3{ 25.f, 21.5f,0.f }, "2");
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("road");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 50.f,-3.f,-16.5f }, glm::vec3{ 25.f, 21.5f,0.f }, "3");
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("road");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 77.5f,-3.f,-37.65f }, glm::vec3{ 30.f, 42.53f, 0.f }, "4");
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("road1");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 102.5f,-3.f,-16.5f }, glm::vec3{ 25.f, 21.5f,0.f }, "5");
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("road");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 127.5f,-3.f,-16.5f }, glm::vec3{ 25.f, 21.5f,0.f }, "-1");
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("road");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -17.5f, 5.f,-38.f }, glm::vec3{ 22.f, 16.f,0.f }, "0");
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("3");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 7.5f, 5.f,-38.f }, glm::vec3{ 22.f, 16.f,0.f }, "1");
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("1");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 29.5f, 5.f,-38.f }, glm::vec3{ 22.f, 16.f,0.f }, "2");
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("2");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 51.5f, 5.f,-38.f }, glm::vec3{ 22.f, 16.f,0.f }, "3");
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("3");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -6.5f, 6.f,-60.f }, glm::vec3{ 22.f, 18.f,0.f }, "1.1");
	Engine::GetObjectManager().GetLastObject()->SetYRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("4");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -3.5f, 6.f,-60.f }, glm::vec3{ 22.f, 18.f,0.f }, "1.2");
	Engine::GetObjectManager().GetLastObject()->SetYRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("4");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -3.5f, 6.f,-58.f }, glm::vec3{ 6.f, 18.f,0.f }, "1.3");
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("4_2");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -5.f, -3.f,-50.5f }, glm::vec3{ 3.f, 13.5f,0.f }, "road");
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("4_1");

	Engine::GetObjectManager().AddObject<BEUPlayer>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 6.f, 6.f,0.f }, "Player", beatEmUpDemoSystem);

	beatEmUpDemoSystem->Init();

	beatEmUpDemoSystem->SpawnEnemy(glm::vec3{ -6.f,0.f,0.f }, glm::vec3{ 6.f, 6.f,0.f }, "Enemy");
}

void BeatEmUpDemo::Update(float dt)
{
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::R))
	{
		Engine::GetGameStateManager().SetGameState(State::RESTART);
	} 
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::W))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::FOWARD, 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::S))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::BACKWARD, 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::A))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::LEFT, 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::D))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::RIGHT, 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::Q))
	{
		if (rand() % 2 == 1)
		{
			beatEmUpDemoSystem->SpawnEnemy(glm::vec3{ 6.f, 0.f, static_cast<float>(rand() % (4 - (-28) + 1) + (-28)) }
			, glm::vec3{ 6.f, 6.f,0.f }, "Enemy");
		}
		else
		{
			beatEmUpDemoSystem->SpawnEnemy(glm::vec3{ -6.f, 0.f, static_cast<float>(rand() % (4 - (-28) + 1) + (-28)) }
			, glm::vec3{ 6.f, 6.f,0.f }, "Enemy");
		}
	}

	beatEmUpDemoSystem->Update(dt);
}

#ifdef _DEBUG
void BeatEmUpDemo::ImGuiDraw(float /*dt*/)
{
	Engine::GetGameStateManager().StateChanger();
	Engine::GetCameraManager().CameraControllerImGui();
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

