//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: VerticesDemo.cpp
#include "VerticesDemo.hpp"
#include "Engine.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "BasicComponents/Sprite.hpp"

#include <iostream>

void VerticesDemo::Init()
{
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample3.jpg");

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1280.f,720.f,0.f }, "0", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 512.f,512.f,0.f }, "1", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(2);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 640.f, 360.f,0.f }, glm::vec3{ 256.f, 128.f,0.f }, "2", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuadLine({ 1.f,0.f,1.f,1.f });

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f, 0.f,0.f }, glm::vec3{ 70.f, 120.f,0.f }, "3", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->LoadAnimation("../Game/assets/player.spt", 3);
}

void VerticesDemo::Update(float dt)
{
	if(Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_1))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<Sprite>()->ChangeTexture(0.f);
	else if(Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_2))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<Sprite>()->ChangeTexture(1.f);
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_3))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<Sprite>()->ChangeTexture(2.f);

	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::UP))
	{
		Engine::Instance().GetCameraManager()->MoveUp(5.f);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::DOWN))
	{
		Engine::Instance().GetCameraManager()->MoveUp(-5.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::LEFT))
	{
		Engine::Instance().GetCameraManager()->MoveRight(-5.f);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		Engine::Instance().GetCameraManager()->MoveRight(5.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::A))
	{
		Engine::Instance().GetCameraManager()->SetRotate(Engine::Instance().GetCameraManager()->GetRotate2D() - 50.f * dt);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::S))
	{
		Engine::Instance().GetCameraManager()->SetRotate(Engine::Instance().GetCameraManager()->GetRotate2D() + 50.f * dt);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::Z))
	{
		Engine::Instance().GetCameraManager()->SetZoom(Engine::Instance().GetCameraManager()->GetZoom() - 5.f * dt);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::X))
	{
		Engine::Instance().GetCameraManager()->SetZoom(Engine::Instance().GetCameraManager()->GetZoom() + 5.f * dt);
	}
	//Engine::Instance().GetObjectManager()->FindObjectWithId(2)->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithId(2)->GetRotate() + 50.f * dt);
	Engine::Instance().GetObjectManager()->FindObjectWithId(2)->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithId(2)->GetRotate() + 50.f * dt);

	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::SPACE))
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->PlayAnimation(0);
}

#ifdef _DEBUG
void VerticesDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
	Engine::GetSoundManager()->MusicPlayerForImGui();
}
#endif

void VerticesDemo::Restart()
{
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
}

void VerticesDemo::End()
{
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
}

