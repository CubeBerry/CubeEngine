//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: main.cpp
#include "Engine.hpp"

#include "VerticesDemo.hpp"
#include "ProceduralMeshes.hpp"
#include "3DPhysicsDemo.hpp"
#include "PocketBallDemo/PocketBallDemo.hpp"
#include "PlatformDemo/PlatformDemo.hpp"
#include "BeatEmUpDemo/BeatEmUpDemo.hpp"
#include "PBR.hpp"

#include "DebugTools.hpp"
#include "Profiler.h"

#undef main

int main(void)
{
#ifdef _DEBUG
	DebugTools::EnableMemoryLeakDetection();
    DebugTools::EnableWriteDump();
    Profiler::GetInstance()->InitStart();
#endif

    Engine& engine = Engine::Instance();
    engine.Init("CubeEngine", 1280, 720, false, WindowMode::NORMAL);
    engine.SetFPS(FrameRate::UNLIMIT);

    //engine.GetSoundManager().LoadSoundFilesFromFolder(L"..\\Game\\assets\\Musics");
    engine.GetSoundManager().LoadSoundFilesFromFolder("../Game/assets/Sounds");

    engine.GetGameStateManager().AddLevel(new ProceduralMeshes);
    engine.GetGameStateManager().AddLevel(new VerticesDemo);
    engine.GetGameStateManager().AddLevel(new PhysicsDemo);
    engine.GetGameStateManager().AddLevel(new PocketBallDemo);
    engine.GetGameStateManager().AddLevel(new PlatformDemo);
    engine.GetGameStateManager().AddLevel(new BeatEmUpDemo);
    engine.GetGameStateManager().AddLevel(new PBR);
    engine.GetGameStateManager().LevelInit(GameLevel::PROCEDURALMESHES);

#ifdef _DEBUG
    Profiler::GetInstance()->InitEnd();
    Profiler::GetInstance()->Start();
#endif
    engine.Update();
#ifdef _DEBUG
    Profiler::GetInstance()->Stop();
    Profiler::GetInstance()->ExitBegin();
#endif
    engine.End();
#ifdef _DEBUG
    Profiler::GetInstance()->ExitEnd();
#endif
}