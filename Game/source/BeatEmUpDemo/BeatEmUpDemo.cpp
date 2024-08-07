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
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 45.f);
	Engine::GetCameraManager().SetFar(90.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,10.f, 30.f });
	Engine::GetCameraManager().UpdaetCameraDirectrion({ 0.f, 10.f });

	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/road.png", "road");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/road1.png", "road1");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/1.png", "1");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/2.png", "2");
	Engine::GetRenderManager()->LoadTexture("../Game/assets/BeatEmUpDemo/3.png", "3");

	beatEmUpDemoSystem = new BeatEmUpDemoSystem();
	beatEmUpDemoSystem->Init();

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

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 51.5f, 5.f,-38.f }, glm::vec3{ 22.f, 16.f,0.f }, "road");
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("3");

	Engine::GetObjectManager().AddObject<BEUPlayer>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 6.f, 6.f,0.f }, "Player", beatEmUpDemoSystem);
	Engine::GetObjectManager().AddObject<BEUEnemy>(glm::vec3{ -6.f,0.f,0.f }, glm::vec3{ 6.f, 6.f,0.f }, "Enemy", beatEmUpDemoSystem);
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
	beatEmUpDemoSystem->Update(dt);
}

#ifdef _DEBUG
void BeatEmUpDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
	Engine::GetGameStateManager().StateChanger();
	Engine::GetSoundManager().MusicPlayerForImGui(0);
	Engine::GetCameraManager().CameraControllerImGui();

	ImGui::Begin("Road");
	float targetP[3] = { Engine::GetObjectManager().FindObjectWithName("road")->GetPosition().x, Engine::GetObjectManager().FindObjectWithName("road")->GetPosition().y, Engine::GetObjectManager().FindObjectWithName("road")->GetPosition().z };
	float targetS[3] = { Engine::GetObjectManager().FindObjectWithName("road")->GetSize().x, Engine::GetObjectManager().FindObjectWithName("road")->GetSize().y, Engine::GetObjectManager().FindObjectWithName("road")->GetSize().z };
	
	ImGui::Text("POS");
	ImGui::InputFloat("XPOS", &targetP[0], 0.5f, 1.0f);
	Engine::GetObjectManager().FindObjectWithName("road")->SetXPosition(targetP[0]);
	ImGui::InputFloat("YPOS", &targetP[1], 0.5f, 1.0f);
	Engine::GetObjectManager().FindObjectWithName("road")->SetYPosition(targetP[1]);
	ImGui::InputFloat("ZPOS", &targetP[2], 0.5f, 1.0f);
	Engine::GetObjectManager().FindObjectWithName("road")->SetZPosition(targetP[2]);

	ImGui::Text("SIZE");
	ImGui::InputFloat("XSIZE", &targetS[0], 0.5f, 1.0f);
	Engine::GetObjectManager().FindObjectWithName("road")->SetXSize(targetS[0]);
	ImGui::InputFloat("YSIZE", &targetS[1], 0.5f, 1.0f);
	Engine::GetObjectManager().FindObjectWithName("road")->SetYSize(targetS[1]);
	ImGui::InputFloat("ZSIZE", &targetS[2], 0.5f, 1.0f);
	Engine::GetObjectManager().FindObjectWithName("road")->SetZSize(targetS[2]);
	ImGui::End();
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

