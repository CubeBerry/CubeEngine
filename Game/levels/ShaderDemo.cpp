#include "ShaderDemo.hpp"
#include "Engine.hpp"
//#include "../Game/FileIO.hpp"

#include <iostream>

void ShaderDemo::Init()
{
	renderManager = Engine::GetVKRenderManager();
	inputManager = Engine::GetInputManager();
	window = Engine::GetWindow();

	std::vector<Vertex> vertices{
		Vertex(glm::vec3(-0.5f, 0.5f, 0), glm::vec3(1.f,0,0), glm::vec2(0, 1.f)),
		Vertex(glm::vec3(0.5f, 0.5f, 0), glm::vec3(0,1.f,0), glm::vec2(1.f, 1.f)),
		Vertex(glm::vec3(0.0, -0.5f, 0), glm::vec3(0,0,1.f), glm::vec2(0.5f, 0)),
	};
	vkVertexBuffer = new VKVertexBuffer(renderManager->GetVkInit(), &vertices);

	std::vector<uint16_t> indices{ 0,1,2 };
	vkIndexBuffer = new VKIndexBuffer(renderManager->GetVkInit(), renderManager->GetCommandPool(), &indices);

	vkUniformBuffer = new VKUniformBuffer<Material>(renderManager->GetVkInit());

	vkTexture = new VKTexture(renderManager->GetVkInit(), renderManager->GetCommandPool());
	vkTexture->LoadTexture("Assets/texture_sample.png");
#ifdef _DEBUG
#endif
}

void ShaderDemo::Update(float /*dt*/)
{
	if (inputManager->IsKeyPressedOnce(KEYBOARDKEYS::NUMBER_2))
	{
		Engine::GetGameStateManager()->ChangeLevel(GameLevel::VERTICESDEMO);
	}
#ifdef _DEBUG
	//imgui.Update();
#endif

}

void ShaderDemo::Draw(float dt)
{
	//vkRenderManager->NewClearColor(window_);
	time += 1.f * dt;
	material.color.r = cos(time);
	material.color.g = sin(time);
	material.color.b = cos(time);
	renderManager->BeginRender<Material>(window, vkUniformBuffer, &material, vkTexture);
	//vkRenderManager->DrawVerticesTriangle(vkVertexBuffer->GetVertexBuffer());
	renderManager->DrawIndicesTriangle(vkVertexBuffer->GetVertexBuffer(), vkIndexBuffer->GetIndexBuffer());
	renderManager->EndRender(window);

#ifdef _DEBUG
	//imgui.Update();
#endif

}

void ShaderDemo::Restart()
{
	End();
	renderManager = Engine::GetVKRenderManager();
	inputManager = Engine::GetInputManager();
	window = Engine::GetWindow();
	std::vector<Vertex> vertices{
		Vertex(glm::vec3(-0.5f, 0.5f, 0), glm::vec3(1.f,0,0), glm::vec2(0, 1.f)),
		Vertex(glm::vec3(0.5f, 0.5f, 0), glm::vec3(0,1.f,0), glm::vec2(1.f, 1.f)),
		Vertex(glm::vec3(0.0, -0.5f, 0), glm::vec3(0,0,1.f), glm::vec2(0.5f, 0)),
	};
	vkVertexBuffer = new VKVertexBuffer(renderManager->GetVkInit(), &vertices);

	std::vector<uint16_t> indices{ 0,1,2 };
	vkIndexBuffer = new VKIndexBuffer(renderManager->GetVkInit(), renderManager->GetCommandPool(), &indices);

	vkUniformBuffer = new VKUniformBuffer<Material>(renderManager->GetVkInit());
}

void ShaderDemo::End()
{
	delete vkVertexBuffer;
	delete vkIndexBuffer;
	delete vkUniformBuffer;
	//delete vkTexture;
	time = 0;
	material.color = { 0.f,0.f,0.f };

	renderManager = nullptr;
	inputManager = nullptr;
	window = nullptr;
#ifdef _DEBUG
	//imgui.End();
#endif
}

