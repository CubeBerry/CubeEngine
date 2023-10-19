#include "Engine.hpp"

#include"levels/VerticesDemo.hpp"

#undef main

int main(void)
{
	Engine& engine = Engine::Instance();
	engine.Init("Vulkan Demo", 640, 480, false, WindowMode::NORMAL);
	engine.SetFPS(FrameRate::FPS_60);

	engine.GetGameStateManager()->AddLevel(new VerticesDemo);
	engine.GetGameStateManager()->LevelInit();

	engine.Update();
	engine.End();
}