#include "VerticesDemo.hpp"
#include "Engine.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "BasicComponents/MaterialComponent.hpp"

#include <iostream>

void VerticesDemo::Init()
{
	std::vector<Vertex> vertices =
	{
	   {glm::vec4(-1.f, 1.f, 1.f, 1.f), { 1.f, 1.f, 1.f, 1.f }, 0, 0.f},
	   {glm::vec4(1.f, 1.f, 1.f, 1.f), { 1.f, 1.f, 1.f, 1.f }, 0, 0.f},
	   {glm::vec4(1.f, -1.f, 1.f, 1.f), { 1.f, 1.f, 1.f, 1.f }, 0, 0.f},
	   {glm::vec4(-1.f, -1.f, 1.f, 1.f), { 1.f, 1.f, 1.f, 1.f }, 0, 0.f}
	};

	std::vector<uint64_t> indices
	{
		0, 1, 2, //first triangle
		2, 3, 0  //second triangle
	};

	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample3.jpg");

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1280.f,720.f,0.f }, "0", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<MaterialComponent>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<MaterialComponent>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 512.f,512.f,0.f }, "1", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<MaterialComponent>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<MaterialComponent>()->AddMeshWithTexture(2);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 256.f,256.f,0.f }, "2", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<MaterialComponent>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<MaterialComponent>()->AddMeshWithVertices(vertices, indices);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 256.f, 256.f,0.f }, glm::vec3{ 256.f, 128.f,0.f }, "3", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<MaterialComponent>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<MaterialComponent>()->AddQuadLine({ 1.f,0.f,1.f,1.f });
}

void VerticesDemo::Update(float dt)
{
	if(Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_1))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<MaterialComponent>()->ChangeTexture(0.f);
	else if(Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_2))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<MaterialComponent>()->ChangeTexture(1.f);
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_3))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<MaterialComponent>()->ChangeTexture(2.f);

	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::UP))
	{
		Engine::Instance().GetCameraManager()->MoveUp(5.f);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::DOWN))
	{
		Engine::Instance().GetCameraManager()->MoveUp(-5.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::LEFT))
	{
		Engine::Instance().GetCameraManager()->MoveRight(-5.f);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		Engine::Instance().GetCameraManager()->MoveRight(5.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::A))
	{
		Engine::Instance().GetCameraManager()->SetRotate(Engine::Instance().GetCameraManager()->GetRotate2D() - 50.f * dt);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::S))
	{
		Engine::Instance().GetCameraManager()->SetRotate(Engine::Instance().GetCameraManager()->GetRotate2D() + 50.f * dt);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::Z))
	{
		Engine::Instance().GetCameraManager()->SetZoom(Engine::Instance().GetCameraManager()->GetZoom() - 5.f * dt);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::X))
	{
		Engine::Instance().GetCameraManager()->SetZoom(Engine::Instance().GetCameraManager()->GetZoom() + 5.f * dt);
	}
	Engine::Instance().GetObjectManager()->FindObjectWithId(2)->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithId(2)->GetRotate() + 50.f * dt);
	Engine::Instance().GetObjectManager()->FindObjectWithId(3)->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithId(3)->GetRotate() + 50.f * dt);
}

void VerticesDemo::Draw(float /*dt*/)
{
	Engine::Engine().GetVKRenderManager()->Render();
}

void VerticesDemo::Restart()
{
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
}

void VerticesDemo::End()
{
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
}

