//Author: DOYEONG LEE
//Project: CubeEngine
//File: BeatEmUpDemoSystem.cpp
#include "BeatEmUpDemo/BeatEmUpDemoSystem.hpp"
#include "Engine.hpp"

#include <iostream>
#include <fstream>

#ifdef _DEBUG
#include "imgui.h"
#endif

void BeatEmUpDemoSystem::Init()
{
}

void BeatEmUpDemoSystem::Update(float /*dt*/)
{
	glm::vec2 viewSize = Engine::GetCameraManager().GetViewSize();
	glm::vec2 center = Engine::GetCameraManager().GetCenter();
}

void BeatEmUpDemoSystem::End()
{
}
