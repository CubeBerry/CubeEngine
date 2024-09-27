//Author: JEYOON YU
//Project: CubeEngine
//File: GLImGuiManager.cpp
#include "GLImGuiManager.hpp"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>

GLImGuiManager::GLImGuiManager(SDL_Window* window_, SDL_GLContext context_)
{
	Initialize(window_, context_);
}

GLImGuiManager::~GLImGuiManager()
{
	Shutdown();
}

void GLImGuiManager::Initialize(SDL_Window* window_, SDL_GLContext context_)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui_ImplSDL2_InitForOpenGL(window_, context_);
	ImGui_ImplOpenGL3_Init();
}

//void GLImGuiManager::FeedEvent(const SDL_Event& event_)
//{
//	ImGui_ImplSDL2_ProcessEvent(&event_);
//}

void GLImGuiManager::Begin()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void GLImGuiManager::End()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	//const ImGuiIO& io = ImGui::GetIO();
	//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//{
	//	ImGui::UpdatePlatformWindows();
	//	ImGui::RenderPlatformWindowsDefault();
	//	SDL_GL_MakeCurrent(sdl_window, gl_context);
	//}
}

void GLImGuiManager::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}
