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

	//Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	//for (int i = 0; i < 3; ++i)
	//	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample3.jpg");
	/*Engine::Engine().GetVKRenderManager()->LoadQuad(glm::vec4(0.f, 0.f, 1.f, 1.f), 1.f);
	Engine::Engine().GetVKRenderManager()->LoadQuad(glm::vec4(0.f, 0.f, 1.f, 1.f), 1.f);
	Engine::Engine().GetVKRenderManager()->LoadQuad(glm::vec4(0.f, 0.f, 1.f, 1.f), 0.f);
	Engine::Engine().GetVKRenderManager()->LoadLineQuad(glm::vec4(0.f, 0.f, 1.f, 1.f));
	Engine::Engine().GetVKRenderManager()->LoadLineQuad(glm::vec4(0.f, 0.f, 1.f, 1.f));
	Engine::Engine().GetVKRenderManager()->LoadQuad(glm::vec4(1.f, 0.f, 1.f, 1.f), 1.f);

	auto matrices = Engine::Engine().GetVKRenderManager()->GetMatrices();
	(*matrices)[0].texIndex = 0.f;
	(*matrices)[1].texIndex = 1.f;
	glm::mat4 modelMatrix(1.0f);
	(*matrices)[5].model = glm::scale(modelMatrix, { 0.5f, 0.5f, 0.f });
	(*matrices)[5].texIndex = 0.f;*/

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1280.f,720.f,0.f }, "0", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<MaterialComponent>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<MaterialComponent>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 256.f,256.f,0.f }, "1", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<MaterialComponent>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<MaterialComponent>()->AddMeshWithTexture(2);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 128.f,64.f,0.f }, "2", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<MaterialComponent>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<MaterialComponent>()->AddMeshWithVertices(vertices, indices);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 256.f,256.f,0.f }, glm::vec3{ 128.f,128.f,0.f }, "3", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<MaterialComponent>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<MaterialComponent>()->AddQuadLine({ 1.f,0.f,1.f,1.f });

}

void VerticesDemo::Update(float dt)
{
	//auto matrices = Engine::Engine().GetVKRenderManager()->GetMatrices();
	////(*matrices)[0].model = glm::mat4(1);
	////(*matrices)[0].texIndex = 0.f;
	//(*matrices)[0].model = glm::translate((*matrices)[0].model, { 0.01f, 0.f, 0.f });
	//if ((*matrices)[0].model[3][0] > 2.f)
	//{
	//	(*matrices)[0].model = glm::mat4(1);
	//	switch (static_cast<int>((*matrices)[0].texIndex))
	//	{
	//	case 0:
	//		(*matrices)[0].texIndex = 1.f;
	//		break;
	//	case 1:
	//		(*matrices)[0].texIndex = 0.f;
	//		break;
	//	}
	//}

	//glm::mat4 modelMatrix(1.0f);
	//(*matrices)[1].model = glm::translate(modelMatrix, { 1.f,1.f,0.f });
	//switch (static_cast<int>((*matrices)[1].texIndex))
	//{
	//case 0:
	//	(*matrices)[1].texIndex = 1.f;
	//	break;
	//case 1:
	//	(*matrices)[1].texIndex = 0.f;
	//	break;
	//}

	//glm::mat4 modelMatrix2(1.0f);
	//(*matrices)[2].model = glm::translate(modelMatrix2, { -1.f,1.f,0.f });

	//glm::mat4 modelMatrix3(1.0f);
	////(*matrices)[3].model = glm::translate(modelMatrix3, { -1.f,-1.f,0.f });
	//(*matrices)[3].model = glm::scale(modelMatrix3, { 0.5f, 0.5f, 0.f });

	//glm::mat4 modelMatrix4(1.0f);
	//(*matrices)[4].model = glm::translate(modelMatrix4, { 1.f,-1.f,0.f });

	//if (Engine::Instance().GetInputManager()->IsKeyPressedOnce(KEYBOARDKEYS::Z))
	//{
	//	glm::mat4 modelMatrix5(1.0f);
	//	(*matrices)[5].texIndex = 1.f;
	//	(*matrices)[5].model = glm::scale(modelMatrix5, { 0.5f, 0.5f, 0.f });
	//}
	//else if (Engine::Instance().GetInputManager()->IsKeyPressedOnce(KEYBOARDKEYS::X))
	//{
	//	glm::mat4 modelMatrix5(1.0f);
	//	(*matrices)[5].texIndex = 0.f;
	//	(*matrices)[5].model = glm::scale(modelMatrix5, { 0.5f, 0.5f, 0.f });
	//}

	switch (static_cast<int>(Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<MaterialComponent>()->GetTextureId()))
	{
	case 0:
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<MaterialComponent>()->ChangeTexture(1.f);
		break;
	case 1:
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<MaterialComponent>()->ChangeTexture(0.f);
		break;
	}

	if(Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_1))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<MaterialComponent>()->ChangeTexture(0.f);
	else if(Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_2))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<MaterialComponent>()->ChangeTexture(1.f);
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_3))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<MaterialComponent>()->ChangeTexture(2.f);

	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::UP))
	{
		Engine::Instance().GetCameraManager()->MoveUp(5.f * dt);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::DOWN))
	{
		Engine::Instance().GetCameraManager()->MoveUp(-5.f * dt);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::LEFT))
	{
		Engine::Instance().GetCameraManager()->MoveRight(-5.f * dt);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		Engine::Instance().GetCameraManager()->MoveRight(5.f * dt);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::A))
	{
		Engine::Instance().GetCameraManager()->SetRotate(Engine::Instance().GetCameraManager()->GetRotate2D() - 5.f);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::D))
	{
		Engine::Instance().GetCameraManager()->SetRotate(Engine::Instance().GetCameraManager()->GetRotate2D() + 5.f);
	}

	Engine::Instance().GetObjectManager()->FindObjectWithId(1)->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetRotate() + 50.f * dt);
	Engine::Instance().GetObjectManager()->FindObjectWithId(2)->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithId(2)->GetRotate() + 500.f * dt);
	Engine::Instance().GetObjectManager()->FindObjectWithId(2)->SetXSize(Engine::Instance().GetObjectManager()->FindObjectWithId(2)->GetSize().x + 1.f * dt);
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

