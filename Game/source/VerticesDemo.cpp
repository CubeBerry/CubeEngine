#include "VerticesDemo.hpp"
#include "Engine.hpp"

#include <iostream>

void VerticesDemo::Init()
{
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
}

void VerticesDemo::Update(float /*dt*/)
{
}

void VerticesDemo::Draw(float /*dt*/)
{
	auto* window = Engine::Engine().GetWindow();
	Engine::Engine().GetVKRenderManager()->Render(window);
}

void VerticesDemo::Restart()
{
}

void VerticesDemo::End()
{
}

