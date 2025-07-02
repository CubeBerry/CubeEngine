// (C) 2025 DigiPen (USA) Corporation

#include "Profiler.h"


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>


#pragma comment(lib, "Dbghelp.lib")


namespace sync_engine
{

    HANDLE symbol_handle;
    void Profiler::initializePlatformDependent()
    {
        const HANDLE currentProcess = GetCurrentProcess();
        DuplicateHandle(currentProcess, currentProcess, currentProcess, &symbol_handle, 0, FALSE, DUPLICATE_SAME_ACCESS);
        SymInitialize(symbol_handle, nullptr, TRUE);
        SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    }


    void Profiler::shutdownPlatformDependent()
    {
        SymCleanup(symbol_handle);
        CloseHandle(symbol_handle);
    }


    std::string get_function_name(void* address)
    {
        static ULONG64 buffer[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64)] = {};

        auto* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        unsigned long long* displacement = nullptr;
        SymFromAddr(symbol_handle, reinterpret_cast<DWORD64>(address), displacement, symbol);
        return { symbol->Name, symbol->NameLen };
    }

}
