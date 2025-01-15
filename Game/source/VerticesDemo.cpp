//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: VerticesDemo.cpp
#include "VerticesDemo.hpp"
#include "Engine.hpp"

#include <iostream>

void VerticesDemo::Init()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::TwoDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 1.f);
	Engine::GetCameraManager().SetFar(45.f);
	Engine::GetCameraManager().SetBaseFov(22.5f);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);

	Engine::GetRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg", "1", true);
	Engine::GetRenderManager()->LoadTexture("../Game/assets/texture_sample.jpg", "2", true);
	Engine::GetRenderManager()->LoadTexture("../Game/assets/PlatformDemo/playerFPS.png", "FPS", true);

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,4.f,-9.f }, glm::vec3{ 16.f,9.f,0.f }, "0", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("1");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -600.f,328.f,0.f }, glm::vec3{ 64.f,64.f,0.f }, "1", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->LoadAnimation("../Game/assets/PlatformDemo/player.spt", "Player");
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->PlayAnimation(1);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->SetSpriteDrawType(SpriteDrawType::UI);

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -568.f,328.f,0.f }, glm::vec3{ 64.f,64.f,0.f }, "2", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->LoadAnimation("../Game/assets/PlatformDemo/player.spt", "Player");
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->PlayAnimation(1);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->SetSpriteDrawType(SpriteDrawType::UI);

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -536.f,328.f,0.f }, glm::vec3{ 64.f,64.f,0.f }, "3", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->LoadAnimation("../Game/assets/PlatformDemo/player.spt", "Player");
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->PlayAnimation(1);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->SetSpriteDrawType(SpriteDrawType::UI);

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,-256.f,0.f }, glm::vec3{ 256.f,256.f,0.f }, "4", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("FPS");
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->SetSpriteDrawType(SpriteDrawType::UI);
}

void VerticesDemo::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);
}

void VerticesDemo::ImGuiDraw(float /*dt*/)
{
	Engine::GetGameStateManager().StateChanger();
	Engine::GetCameraManager().CameraControllerImGui();
	Engine::GetObjectManager().ObjectControllerForImGui();
}

void VerticesDemo::Restart()
{
	End();
}

void VerticesDemo::End()
{
	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().End();
}
