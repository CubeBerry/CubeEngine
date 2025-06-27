#include "Profiler.h"
#include "Internal.h"

namespace ProfilerExample {

	void DoNothing()
	{
		//NO OP;
	}

} // end namespace Profiler

Node::Node(void* address, Node* parent)
{
	this->address = address;
	this->parent = parent;
	this->recursion = 0;
	this->startCycle = 0;
	this->totalCycle = 0;
	this->callCount = 0;
}

Node::~Node()
{
	for (Node* child : children)
	{
		delete child;
	}
	children.clear();
}

void Profiler::InitEnd()
{
	// Finalize the profiler initialization
	// This is a placeholder for actual finalization code
}

void Profiler::Start()
{
	// Start the profiler
	// This is a placeholder for actual start code
	isEnabled = true;
}

void Profiler::Stop()
{
	// Stop the profiler
	// This is a placeholder for actual stop code
	isEnabled = false;
}

void Profiler::FrameBegin(signed int frameCount)
{
	// Begin a new frame
	// This is a placeholder for actual frame begin code
	frameCount;
}

void Profiler::FrameEnd(signed int frameCount)
{
	// End the current frame
	// This is a placeholder for actual frame end code
	frameCount;
}

void Profiler::KeyPressed(unsigned char key)
{
	// Handle key press events
	// This is a placeholder for actual key press handling code
	key;
}

Node* Profiler::FindChild(Node* parent, void* address)
{
	auto it = std::find_if(parent->children.begin(), parent->children.end(),
		[&](const Node* child) { return child->address == address; });
	if (it != parent->children.end()) return *it;

	// If not found, create a new child node
	parent->children.emplace_back(new Node{ address, parent });
	return parent->children.back();
}
