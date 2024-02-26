//Author: DOYEONG LEE
//Project: CubeEngine
//File: main.cpp
#include "Engine.hpp"

#include "VerticesDemo.hpp"
#include "PocketBallDemo/PocketBallDemo.hpp"
#include "PlatformDemo/PlatformDemo.hpp"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#if _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define malloc(s) _malloc_dbg(s,_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#undef main

int main(void)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(3367);
    //_crtBreakAlloc = 3367;

    Engine& engine = Engine::Instance();
    engine.Init("Vulkan Demo", 1280, 720, false, WindowMode::NORMAL);
    engine.SetFPS(FrameRate::FPS_60);

    engine.GetSoundManager()->LoadMusicFilesFromFolder(L"..\\Game\\assets\\Musics");
    engine.GetSoundManager()->LoadSoundFilesFromFolder("../Game/assets/Sounds");

    engine.GetGameStateManager()->AddLevel(new PocketBallDemo);
    engine.GetGameStateManager()->AddLevel(new PlatformDemo);
    engine.GetGameStateManager()->LevelInit(GameLevel::PLATFORMDEMO);

    engine.Update();
    engine.End();

    //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();
    //_CrtSetBreakAlloc(3367);
}