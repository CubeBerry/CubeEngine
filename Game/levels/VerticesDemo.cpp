#include "VerticesDemo.hpp"
#include "Engine.hpp"
//#include "../Game/FileIO.hpp"

#include <iostream>

void VerticesDemo::Init()
{
	renderManager = Engine::GetVKRenderManager();
	inputManager = Engine::GetInputManager();
	window = Engine::GetWindow();

	std::vector<Vertex> vertices{
		Vertex(glm::vec3(-0.5f, 0.5f, 0), glm::vec3(1.f,0,0), glm::vec2(0, 1.f)),
		Vertex(glm::vec3(0.5f, 0.5f, 0), glm::vec3(0,1.f,0), glm::vec2(1.f, 1.f)),
		Vertex(glm::vec3(0.0, -0.5f, 0), glm::vec3(0,0,1.f), glm::vec2(0.5f, 0)),
	};
	std::vector<Vertex> vertices1{
		Vertex(glm::vec3(-1.f, -1.f, 0),glm::vec3(1.f), glm::vec2(0, 0)),
		Vertex(glm::vec3(-0.5f, -1.f, 0), glm::vec3(1.f), glm::vec2(1.f, 0)),
		Vertex(glm::vec3(-0.5f, -0.5f, 0), glm::vec3(1.f), glm::vec2(1.f, 1.f)),
		Vertex(glm::vec3(-1.f, -0.5f, 0), glm::vec3(1.f), glm::vec2(0, 1.f)),
	};
	std::vector<Vertex> vertices2{
		Vertex(glm::vec3(0.75f, 0.75f, 0.5f),glm::vec3(1.f), glm::vec2(0, 0)),
		Vertex(glm::vec3(1.f, 0.75f, 0.5f), glm::vec3(1.f), glm::vec2(1.f, 0)),
		Vertex(glm::vec3(1.f, 1.f, 0.5f), glm::vec3(1.f), glm::vec2(1.f, 1.f)),
		Vertex(glm::vec3(0.75f, 1.f, 0.5f), glm::vec3(1.f), glm::vec2(0, 1.f)),
		Vertex(glm::vec3(0.75f, 0.75f, -0.5f),glm::vec3(1.f), glm::vec2(0, 0)),
		Vertex(glm::vec3(1.f, 0.75f, -0.5f), glm::vec3(1.f), glm::vec2(1.f, 0)),
		Vertex(glm::vec3(1.f, 1.f, -0.5f), glm::vec3(1.f), glm::vec2(1.f, 1.f)),
		Vertex(glm::vec3(0.75f, 1.f, -0.5f), glm::vec3(1.f), glm::vec2(0, 1.f)),
	};
	std::vector<uint16_t> indices{
		0,1,2
	};
	std::vector<uint16_t> indices1{
		0, 1, 2,
		2, 3, 0
	};
	std::vector<uint16_t> indices2{
	  0, 2, 3, 0, 3, 1,
	  2, 6, 7, 2, 7, 3,
	  6, 4, 5, 6, 5, 7,
	  4, 0, 1, 4, 1, 5,
	  0, 4, 6, 0, 6, 2,
	  1, 5, 7, 1, 7, 3,
	};

	vkUniformBuffer = new VKUniformBuffer<Material>(renderManager->GetVkInit());

	Mesh temp;
	temp.vkVertexBuffer = new VKVertexBuffer(renderManager->GetVkInit(), &vertices);
	temp.vkIndexBuffer = new VKIndexBuffer(renderManager->GetVkInit(), renderManager->GetCommandPool(), &indices);

	Mesh temp1;
	temp1.vkVertexBuffer = new VKVertexBuffer(renderManager->GetVkInit(), &vertices1);
	temp1.vkIndexBuffer = new VKIndexBuffer(renderManager->GetVkInit(), renderManager->GetCommandPool(), &indices1);

	Mesh temp2;
	temp2.vkVertexBuffer = new VKVertexBuffer(renderManager->GetVkInit(), &vertices2);
	temp2.vkIndexBuffer = new VKIndexBuffer(renderManager->GetVkInit(), renderManager->GetCommandPool(), &indices2);

	vkTexture = new VKTexture(renderManager->GetVkInit(), renderManager->GetCommandPool());
	vkTexture->LoadTexture("Assets/texture_sample.png");

	meshs.push_back(std::move(temp));
	meshs.push_back(std::move(temp1));
	meshs.push_back(std::move(temp2));
#ifdef _DEBUG
#endif
}

void VerticesDemo::Update(float /*dt*/)
{
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
	//vkRenderManager->NewClearColor(window_);
	renderManager->BeginRender<Material>(window, vkUniformBuffer, &material, vkTexture);
	//vkRenderManager->DrawVerticesTriangle(vkVertexBuffer->GetVertexBuffer());
	for (auto& mesh : meshs)
	{
		renderManager->DrawIndicesTriangle(mesh.vkVertexBuffer->GetVertexBuffer(), mesh.vkIndexBuffer->GetIndexBuffer());
	}
	renderManager->EndRender(window);

#ifdef _DEBUG
	//imgui.Update();
#endif

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

