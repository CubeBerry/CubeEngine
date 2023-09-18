#include "Engine.hpp"

#undef main

int main(void)
{
	Engine& engine = Engine::Instance();
	engine.Init();
	engine.Update();
	engine.End();
}