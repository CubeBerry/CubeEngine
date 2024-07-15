//Author: JEYOON YU
//Project: CubeEngine
//File: OpenGLDemo.cpp
#include "OpenGLDemo.hpp"
#include "Engine.hpp"

void OpenGLDemo::Init()
{
	Engine::GetRenderManager()->LoadTexture("../Game/assets/texture_sample.png", "White");

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 640.f,360.f,0.f }, glm::vec3{ 1280.f, 720.f,0.f }, "Wall", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("White");
}

void OpenGLDemo::Update(float /*dt*/)
{
}

#ifdef _DEBUG
void OpenGLDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
	Engine::GetSoundManager().MusicPlayerForImGui(0);
	Engine::GetGameStateManager().StateChanger();
}
#endif

void OpenGLDemo::Restart()
{
}

void OpenGLDemo::End()
{
}
