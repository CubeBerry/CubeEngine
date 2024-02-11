//Author: DOYEONG LEE
//Project: CubeEngine
//File: main.cpp
#include "Engine.hpp"

#include "VerticesDemo.hpp"
#include "PocketBallDemo/PocketBallDemo.hpp"
#include "PlatformDemo/PlatformDemo.hpp"

#undef main

int main(void)
{
    Engine& engine = Engine::Instance();
    engine.Init("Vulkan Demo", 1280, 720, false, WindowMode::NORMAL);
    engine.SetFPS(FrameRate::FPS_60);

    engine.GetSoundManager()->LoadMusicFilesFromFolder(L"..\Game\assets\Musics");
    engine.GetSoundManager()->LoadSoundFilesFromFolder("../Game/assets/Sounds");

    engine.GetGameStateManager()->AddLevel(new VerticesDemo);
    engine.GetGameStateManager()->AddLevel(new PocketBallDemo);
    engine.GetGameStateManager()->AddLevel(new PlatformDemo);
    engine.GetGameStateManager()->LevelInit(GameLevel::PLATFORMDEMO);

    engine.Update();
    engine.End();
}