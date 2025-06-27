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
__declspec(thread) Node* tl_current = nullptr;

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
	// This function is called when entering a function
	// This is a placeholder for actual profiling code
	if (!Profiler::GetInstance().isEnabled) return;
	std::lock_guard<std::mutex> lock(Profiler::m_mutex);

	Node* current = Profiler::GetInstance().current;
	if (!current) current = Profiler::GetInstance().root;
	if (current->address == address)
	{
		current->recursion++;
		current->callCount++;
		return;
	}

	Profiler::GetInstance().isEnabled = false;
	current = Profiler::GetInstance().FindChild(current, address);
	Profiler::GetInstance().isEnabled = true;
	current->startCycle = __rdtsc();
	current->callCount++;

	Profiler::GetInstance().current = current;
}

extern "C" void ProfileExit(void* /*address*/)
{
	// This function is called when exiting a function
	// This is a placeholder for actual profiling code
	if (!Profiler::GetInstance().isEnabled) return;
	std::lock_guard<std::mutex> lock(Profiler::m_mutex);

	Node* current = Profiler::GetInstance().current;
	if (!current) return;
	if (current->recursion > 0)
	{
		current->recursion--;
		return;
	}

	current->totalCycle += __rdtsc() - current->startCycle;
	if (current->parent) Profiler::GetInstance().current = current->parent;
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
