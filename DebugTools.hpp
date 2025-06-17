//Author: JEYOON YU
//Project: CubeEngine
//File: DebugTools.hpp
#pragma once

#ifdef _DEBUG
//--------------------Dump Memory Leaks--------------------//
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define malloc(s) _malloc_dbg(s,_NORMAL_BLOCK,__FILE__,__LINE__)

//Call _CrtDumpMemoryLeaks after main has returned and before program terminates.
inline struct AtExit
{
    ~AtExit() { _CrtDumpMemoryLeaks(); }
} doAtExit;

//--------------------Dump Writer--------------------//
#pragma comment (lib, "dbghelp.lib")
#define CASE(X) case X: return #X
#include <Windows.h>
#include <DbgHelp.h>

inline const char* TranslateExceptionCode(long exceptionCode)
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

inline LONG WINAPI WriteDump(EXCEPTION_POINTERS* pException)
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

namespace DebugTools
{
	//--------------------Dump Memory Leaks--------------------//
	inline void EnableMemoryLeakDetection()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(341);
		//_crtBreakAlloc = 157;
		//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	}

    //--------------------Dump Writer--------------------//
	inline void EnableWriteDump()
	{
        ULONG size{ 1024 * 16 };
        SetThreadStackGuarantee(&size);
        SetUnhandledExceptionFilter(WriteDump);
	}
}
#endif
