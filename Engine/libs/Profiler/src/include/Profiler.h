#pragma once

#include "Profiler_API.h"

#include <list>
#include <mutex>

namespace ProfilerExample {

    void PROFILER_API DoNothing();

} // end namespace Profiler

class Node
{
public:
	Node() = default;
	Node(void* address, Node* parent);
	~Node();

	void* address{ nullptr };
	Node* parent{ nullptr };
	std::list<Node*> children;
	unsigned long long recursion{ 0 };
	unsigned long long startCycle{ 0 };
	unsigned long long totalCycle{ 0 };
	unsigned long long callCount{ 0 };
};

class Profiler
{
public:
	static Profiler* GetInstance();

	//std::mutex m_mutex;

	static bool isEnabled;
	Node* root{ nullptr };
	Node* current{ nullptr };

	void InitStart();
	void InitEnd();

	void ExitBegin();
	void ExitEnd();

	void Start();
	void Stop();

	void FrameBegin(signed int frameCount);
	void FrameEnd(signed int frameCount);

	void KeyPressed(unsigned char key);

	// Helper Functions
	Node* FindChild(Node* parent, void* address);
private:
	//static Profiler* instance;
	static std::mutex instanceMutex;

	Profiler() = default;
	~Profiler() = default;
};

extern "C" void ProfileEnter(void* address);
extern "C" void ProfileExit(void* address);
