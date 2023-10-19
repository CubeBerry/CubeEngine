#include "VerticesDemo.hpp"
#include "Engine.hpp"

#include <iostream>

void VerticesDemo::Init()
{
	renderManager = Engine::GetVKRenderManager();
	window = Engine::GetWindow();

	renderManager->LoadTexture("Assets/texture_sample.png");
}

void VerticesDemo::Update(float /*dt*/)
{
}

void VerticesDemo::Draw(float /*dt*/)
{
	renderManager->Render(window);
}

void VerticesDemo::Restart()
{
}

void VerticesDemo::End()
{
}

