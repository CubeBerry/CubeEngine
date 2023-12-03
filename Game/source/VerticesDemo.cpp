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
	   {glm::vec4(-1.f, -1.f, 1.f, 1.f), { 1.f, 1.f, 1.f, 1.f }, 0, 0.f},
	   {glm::vec4(1.f, -1.f, 1.f, 1.f), { 1.f, 1.f, 1.f, 1.f }, 0, 0.f},
	   {glm::vec4(1.f, 1.f, 1.f, 1.f), { 1.f, 1.f, 1.f, 1.f }, 0, 0.f}
	};

	std::vector<uint64_t> indices
	{
		0, 1, 2, //first triangle
		2, 3, 0  //second triangle
	};

	std::vector<Vertex> vertices1 =
	{
	   {glm::vec4(-1.f, 1.f, 1.f, 1.f), { 0.f, 1.f, 1.f, 1.f }, 0, 0.f},
	   {glm::vec4(-1.f, -1.f, 1.f, 1.f), { 1.f, 0.f, 1.f, 1.f }, 0, 0.f},
	   {glm::vec4(1.f, -1.f, 1.f, 1.f), { 1.f, 1.f, 0.f, 1.f }, 0, 0.f}
	};

	std::vector<uint64_t> indices1
	{
		0, 1, 2, 0
	};

	//Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	//for (int i = 0; i < 3; ++i)
	//	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
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

	objects.push_back(new Object({ 0.f,0.f,0.7f }, { 512.f,512.f,0.f }, "0", ObjectType::NONE));
	objects.at(0)->AddComponent<MaterialComponent>();
	objects.at(0)->GetComponent<MaterialComponent>()->AddMeshWithTexture(0);

	objects.push_back(new Object({ 0.f,0.f,0.5f }, { 256.f,256.f,0.f }, "1", ObjectType::NONE));
	objects.at(1)->AddComponent<MaterialComponent>();
	objects.at(1)->GetComponent<MaterialComponent>()->AddMeshWithTexture(1);

	objects.push_back(new Object({ 0.f,0.f,0.3f }, { 128.f,64.f,0.f }, "2", ObjectType::NONE));
	objects.at(2)->AddComponent<MaterialComponent>();
	objects.at(2)->GetComponent<MaterialComponent>()->AddMeshWithVertices(vertices, indices);

	objects.push_back(new Object({ 256.f,256.f,0.3f }, { 128.f,128.f,0.f }, "3", ObjectType::NONE));
	objects.at(3)->AddComponent<MaterialComponent>();
	objects.at(3)->GetComponent<MaterialComponent>()->AddQuadLine({ 1.f,0.f,1.f,1.f });

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

	auto matrices = Engine::Engine().GetVKRenderManager()->GetMatrices();
	switch (static_cast<int>((*matrices)[1].texIndex))
	{
	case 0:
		(*matrices)[1].texIndex = 1.f;
		break;
	case 1:
		(*matrices)[1].texIndex = 0.f;
		break;
	
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::LEFT))
	{
		objects.at(0)->SetXPosition(objects.at(0)->GetPosition().x - 50.f);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		objects.at(0)->SetXPosition(objects.at(0)->GetPosition().x + 50.f);
	}
	objects.at(1)->SetRotate(objects.at(1)->GetRotate() + 50.f * dt);
	objects.at(2)->SetRotate(objects.at(2)->GetRotate() + 500.f * dt);
	objects.at(2)->SetXSize(objects.at(2)->GetSize().x + 1.f * dt);
	for (auto obj : objects)
	{
		obj->Update(dt);
	}
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

