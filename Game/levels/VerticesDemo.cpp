#include "VerticesDemo.hpp"
#include "Engine.hpp"
//#include "../Game/FileIO.hpp"

#include <iostream>

void VerticesDemo::Init()
{
	renderManager = Engine::GetVKRenderManager();
	inputManager = Engine::GetInputManager();
	window = Engine::GetWindow();

	renderManager->LoadTexture("Assets/texture_sample.png");
}

void VerticesDemo::Update(float /*dt*/)
{
	if (inputManager->IsKeyPressedOnce(KEYBOARDKEYS::ESCAPE))
	{
		SDL_Event quitEvent;
		quitEvent.type = SDL_QUIT;
		SDL_PushEvent(&quitEvent);
	}
	if (inputManager->IsKeyPressedOnce(KEYBOARDKEYS::R))
	{
		Engine::GetGameStateManager()->RestartLevel();
	}
	if (inputManager->IsKeyPressedOnce(KEYBOARDKEYS::NUMBER_1))
	{
		Engine::GetGameStateManager()->ChangeLevel(GameLevel::SHADERDEMO);
	}
#ifdef _DEBUG
	//imgui.Update();
#endif

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
	for (auto& mesh : meshs)
	{
		delete mesh.vkVertexBuffer;
		delete mesh.vkIndexBuffer;
	}
	meshs.clear();
	delete vkTexture;
	delete vkUniformBuffer;

	renderManager = nullptr;
	inputManager = nullptr;
	window = nullptr;
#ifdef _DEBUG
	//imgui.End();
#endif
}

