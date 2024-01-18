//Author: DOYEONG LEE
//Project: CubeEngine
//File: main.cpp
#include "Engine.hpp"

#include "VerticesDemo.hpp"

#undef main

int main(void)
{
	Engine& engine = Engine::Instance();
	engine.Init("Vulkan Demo", 1280, 720, false, WindowMode::NORMAL);
	engine.SetFPS(FrameRate::FPS_60);

	engine.GetGameStateManager()->AddLevel(new VerticesDemo);
	engine.GetGameStateManager()->LevelInit();

	engine.Update();
	engine.End();
}