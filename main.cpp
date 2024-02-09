//Author: DOYEONG LEE
//Project: CubeEngine
//File: main.cpp
#include "Engine.hpp"

#include "VerticesDemo.hpp"
#include "PocketBallDemo/PocketBallDemo.hpp"


#undef main

int main(void)
{
	Engine& engine = Engine::Instance();
	engine.Init("Vulkan Demo", 1280, 720, false, WindowMode::NORMAL);
	engine.SetFPS(FrameRate::FPS_60);

	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/Ball.png");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/Ball1.png");

	engine.GetSoundManager()->LoadMusicFilesFromFolder(L"..\\Game\\assets\\Musics");
	engine.GetSoundManager()->LoadSoundFilesFromFolder("../Game/assets/Sounds");

	engine.GetGameStateManager()->AddLevel(new VerticesDemo);
	engine.GetGameStateManager()->AddLevel(new PocketBallDemo);
	engine.GetGameStateManager()->LevelInit(GameLevel::POCKETBALL);

	engine.Update();
	engine.End();
}