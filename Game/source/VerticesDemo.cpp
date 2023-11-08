#include "VerticesDemo.hpp"
#include "Engine.hpp"

#include <iostream>

void VerticesDemo::Init()
{
	//Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	for (int i = 0; i < 3; ++i)
		Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
}

void VerticesDemo::Update(float /*dt*/)
{
}

void VerticesDemo::Draw(float /*dt*/)
{
	Engine::Engine().GetVKRenderManager()->Render();
}

void VerticesDemo::Restart()
{
}

void VerticesDemo::End()
{
}

