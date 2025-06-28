#include "Profiler.h"
#include "Internal.h"

#include <fstream>

#include <intrin.h>
#pragma intrinsic(__rdtsc)

#define NOMINMAX
#include <Windows.h>
#pragma comment (lib, "dbghelp.lib")
#include <DbgHelp.h>

//Profiler* Profiler::instance = nullptr;
std::mutex Profiler::m_mutex;
__declspec(thread) bool tl_isInProfiler{ false };
__declspec(thread) Node* tl_current { nullptr };

int getValue()
{
	return 1;
}

static PSYMBOL_INFO GetSymbol(DWORD64 address, PSYMBOL_INFO buff)
{
	PDWORD64 displacement = 0;
	PSYMBOL_INFO symbol = (PSYMBOL_INFO)buff;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol->MaxNameLen = MAX_SYM_NAME;
	SymFromAddr(GetCurrentProcess(), address, displacement, symbol);
	return symbol;
}

void LogCSV(std::ofstream& logFile, const Node* node)
{
	if (!node) return;

	if (node->address && node->parent)
	{
		ULONG64 buff[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
		PSYMBOL_INFO symbol = GetSymbol((DWORD64)node->address, (PSYMBOL_INFO)buff);

		logFile << "0x" << std::hex << node->address << ','
			<< '\"' << symbol->Name << '\"' << ','
			<< std::dec << node->totalCycle << ','
			<< node->recursion << ','
			<< node->callCount << ','
			<< "0x" << std::hex << (node->parent ? node->parent->address : 0) << '\n';
	}

	for (Node* child : node->children)
	{
		LogCSV(logFile, child);
	}
}

extern "C" void ProfileEnter(void* address)
{
	if (tl_isInProfiler) return;

	struct ProfilerGuard
	{
		ProfilerGuard() { tl_isInProfiler = true; }
		~ProfilerGuard() { tl_isInProfiler = false; }
	} guard;

	// This function is called when entering a function
	// This is a placeholder for actual profiling code
	if (!Profiler::GetInstance().isEnabled) return;
	
	if (tl_current == nullptr) tl_current = Profiler::GetInstance().root;

	if (tl_current->address == address)
	{
		Profiler::GetInstance().isEnabled = false;
		std::lock_guard<std::mutex> lock(Profiler::m_mutex);
		Profiler::GetInstance().isEnabled = true;
		tl_current->recursion++;
		tl_current->callCount++;
		return;
	}

	Node* child;
	{
		Profiler::GetInstance().isEnabled = false;
		std::lock_guard<std::mutex> lock(Profiler::m_mutex);
		child = Profiler::GetInstance().FindChild(tl_current, address);
		Profiler::GetInstance().isEnabled = true;
		tl_current->callCount++;
	}

	tl_current->startCycle = __rdtsc();

	tl_current = child;
}

extern "C" void ProfileExit(void* /*address*/)
{
	if (tl_isInProfiler) return;

	struct ProfilerGuard
	{
		ProfilerGuard() { tl_isInProfiler = true; }
		~ProfilerGuard() { tl_isInProfiler = false; }
	} guard;

	// This function is called when exiting a function
	// This is a placeholder for actual profiling code
	if (!Profiler::GetInstance().isEnabled) return;

	Node* current = tl_current;
	if (!current) return;
	if (current->recursion > 0)
	{
		Profiler::GetInstance().isEnabled = false;
		std::lock_guard<std::mutex> lock(Profiler::m_mutex);
		Profiler::GetInstance().isEnabled = true;
		current->recursion--;
		return;
	}

	unsigned long long endCycle = __rdtsc();
	{
		Profiler::GetInstance().isEnabled = false;
		std::lock_guard<std::mutex> lock(Profiler::m_mutex);
		Profiler::GetInstance().isEnabled = true;
		current->totalCycle += endCycle - current->startCycle;
	}

	tl_current = current->parent;
}

Profiler& Profiler::GetInstance()
{
	//if (instance == nullptr)
	//{
	//	//std::lock_guard<std::mutex> lock(instanceMutex);
	//	if (instance == nullptr) instance = new Profiler;
	//}
	//return instance;

	// @TODO Need to make sure this is thead-safe
	static Profiler instance;
	return instance;
}

void Profiler::InitStart()
{
	// Initialize the profiler
	// This is a placeholder for actual initialization code
	root = new Node{ nullptr, nullptr };
	current = root;

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	if (!SymInitialize(GetCurrentProcess(), NULL, true)) {
		return;
	}
}

void Profiler::ExitBegin()
{
	// Prepare for exit
	// This is a placeholder for actual exit preparation code
	std::ofstream file("ProfileReport.csv", std::ios::app);
	file << "RIP, Function Name, Total Cycles, Recursion Count, Call Count, Parent Function Address" << '\n';
	file.close();
}

void Profiler::ExitEnd()
{
	// Finalize the exit process
	// This is a placeholder for actual exit finalization code
	std::ofstream logFile("ProfileReport.csv", std::ios::app);
	LogCSV(logFile, root);
	logFile.close();

	delete root;
	root = nullptr;
	current = nullptr;

	SymCleanup(GetCurrentProcess());
}
