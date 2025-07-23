//Author: JEYOON YU
//Project: CubeEngine
//File: DebugTools.cpp
#include "DebugTools.hpp"

void DebugTools::EnableMemoryLeakDetection()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(368);
	//_crtBreakAlloc = 157;
	//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
}