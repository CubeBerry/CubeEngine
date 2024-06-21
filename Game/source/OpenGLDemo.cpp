//Author: JEYOON YU
//Project: CubeEngine
//File: OpenGLDemo.cpp
#include "OpenGLDemo.hpp"
#include "Engine.hpp"

void OpenGLDemo::Init()
{
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
