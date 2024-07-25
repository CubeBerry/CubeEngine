//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: VerticesDemo.cpp
#include "VerticesDemo.hpp"
#include "Engine.hpp"

#include <iostream>

void VerticesDemo::Init()
{
	Engine::Instance().GetCameraManager().Init(Engine::Instance().GetWindow().GetWindowSize(), CameraType::ThreeDimension, 45.f);
	Engine::Instance().GetCameraManager().SetCameraSensitivity(10.f);

	Engine::Instance().GetRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg", "1");
	Engine::Instance().GetRenderManager()->LoadTexture("../Game/assets/texture_sample.jpg", "2");

	Engine::Instance().GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,4.f,-9.f }, glm::vec3{ 16.f,9.f,0.f }, "0", ObjectType::NONE);
	Engine::Instance().GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("1");

	Engine::Instance().GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,-2.f }, glm::vec3{ 0.2f,0.2f,0.f }, "1", ObjectType::NONE);
	Engine::Instance().GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager().GetLastObject()->GetComponent<Sprite>()->LoadAnimation("../Game/assets/PlatformDemo/player.spt", "Player");
	Engine::Instance().GetObjectManager().GetLastObject()->GetComponent<Sprite>()->PlayAnimation(1);
}

void VerticesDemo::Update(float dt)
{
	Input(dt);
}

#ifdef _DEBUG
void VerticesDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
	Engine::GetGameStateManager().StateChanger();
}
#endif

void VerticesDemo::Restart()
{
	Engine::Instance().GetObjectManager().DestroyAllObjects();
}

void VerticesDemo::End()
{
	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}

void VerticesDemo::Input(float dt)
{
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::W))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::FOWARD, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::S))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::BACKWARD, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::A))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::LEFT, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::D))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::RIGHT, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::SPACE))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::UP, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::LSHIFT))
	{
		Engine::GetCameraManager().MoveCameraPos(CameraMoveDir::DOWN, 5.f * dt);
	}
	//if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::Q))
	//{
	//	if (SDL_GetRelativeMouseMode() == SDL_FALSE)
	//	{
	//		Engine::Instance().GetInputManager().SetRelativeMouseMode(true);
	//	}
	//	else
	//	{
	//		Engine::Instance().GetInputManager().SetRelativeMouseMode(false);
	//	}
	//}
	if (Engine::GetInputManager().GetMouseWheelMotion().y != 0.f)
	{
		Engine::GetCameraManager().SetZoom(Engine::GetCameraManager().GetZoom() + Engine::GetInputManager().GetMouseWheelMotion().y);
	}
	if (Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::LEFT))
	{
		Engine::GetCameraManager().UpdaetCameraDirectrion(Engine::Instance().GetInputManager().GetRelativeMouseState() * dt);
	}

	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::DOWN))
	{
		Engine::Instance().GetObjectManager().GetLastObject()->SetZPosition(Engine::Instance().GetObjectManager().GetLastObject()->GetPosition().z + 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::UP))
	{
		Engine::Instance().GetObjectManager().GetLastObject()->SetZPosition(Engine::Instance().GetObjectManager().GetLastObject()->GetPosition().z - 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::LEFT))
	{
		Engine::Instance().GetObjectManager().GetLastObject()->SetXPosition(Engine::Instance().GetObjectManager().GetLastObject()->GetPosition().x - 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		Engine::Instance().GetObjectManager().GetLastObject()->SetXPosition(Engine::Instance().GetObjectManager().GetLastObject()->GetPosition().x + 5.f * dt);
	}
}
