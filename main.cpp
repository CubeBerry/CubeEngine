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

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define malloc(s) _malloc_dbg(s,_NORMAL_BLOCK,__FILE__,__LINE__)

//Call _CrtDumpMemoryLeaks after main has returned and before program terminates.
struct AtExit
{
    ~AtExit() { _CrtDumpMemoryLeaks(); }
} doAtExit;
#endif

#undef main

//Dump Writer
#pragma comment (lib, "dbghelp.lib")
#define CASE(X) case X: return #X
#include <Windows.h>
#include <DbgHelp.h>

const char* TranslateExceptionCode(long exceptionCode)
{
    switch (exceptionCode)
    {
        CASE(STATUS_STACK_OVERFLOW);
        CASE(STATUS_FLOAT_UNDERFLOW);
        CASE(STATUS_ACCESS_VIOLATION);
    default:
        return "UNKNOWN";
    }
}

LONG WINAPI WriteDump(EXCEPTION_POINTERS* pException)
{
    std::cerr << TranslateExceptionCode(pException->ExceptionRecord->ExceptionCode) << '\n';

    MINIDUMP_EXCEPTION_INFORMATION exceptionInformation{ GetCurrentThreadId(), pException, FALSE };
    HANDLE createFile = CreateFileA("crash_dump.dmp", GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        createFile,
        MINIDUMP_TYPE(MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithCodeSegs | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithUnloadedModules | MiniDumpWithFullMemory),
        &exceptionInformation,
        NULL,
        NULL
    );
    return EXCEPTION_CONTINUE_SEARCH;
}

int main(void)
{
#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(341);
    //_crtBreakAlloc = 157;
#endif

    //Wrtie Dump
    ULONG size{ 1024 * 16 };
    SetThreadStackGuarantee(&size);
    SetUnhandledExceptionFilter(WriteDump);

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

    engine.Update();
    engine.End();

    //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    //_CrtDumpMemoryLeaks();
}