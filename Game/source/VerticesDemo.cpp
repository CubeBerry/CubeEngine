#include "VerticesDemo.hpp"
#include "Engine.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

void VerticesDemo::Init()
{
	//Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	//for (int i = 0; i < 3; ++i)
	//	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
	Engine::Engine().GetVKRenderManager()->LoadQuad(glm::vec4(0.f, 0.f, 1.f, 1.f), 1.f);
	Engine::Engine().GetVKRenderManager()->LoadQuad(glm::vec4(0.f, 0.f, 1.f, 1.f), 1.f);
	Engine::Engine().GetVKRenderManager()->LoadQuad(glm::vec4(0.f, 0.f, 1.f, 1.f), 0.f);
	Engine::Engine().GetVKRenderManager()->LoadLineQuad(glm::vec4(0.f, 0.f, 1.f, 1.f));
	Engine::Engine().GetVKRenderManager()->LoadLineQuad(glm::vec4(0.f, 0.f, 1.f, 1.f));

	auto matrices = Engine::Engine().GetVKRenderManager()->GetMatrices();
	(*matrices)[0].texIndex = 0.f;
	(*matrices)[1].texIndex = 1.f;
}

void VerticesDemo::Update(float /*dt*/)
{
	auto matrices = Engine::Engine().GetVKRenderManager()->GetMatrices();
	//(*matrices)[0].model = glm::mat4(1);
	//(*matrices)[0].texIndex = 0.f;
	(*matrices)[0].model = glm::translate((*matrices)[0].model, {0.01f, 0.f, 0.f});
	if ((*matrices)[0].model[3][0] > 2.f)
	{
		(*matrices)[0].model = glm::mat4(1);
		switch (static_cast<int>((*matrices)[0].texIndex))
		{
		case 0:
			(*matrices)[0].texIndex = 1.f;
			break;
		case 1:
			(*matrices)[0].texIndex = 0.f;
			break;
		}
	}

	glm::mat4 modelMatrix(1.0f);
	(*matrices)[1].model = glm::translate(modelMatrix, { 1.f,1.f,0.f });
	switch (static_cast<int>((*matrices)[1].texIndex))
	{
	case 0:
		(*matrices)[1].texIndex = 1.f;
		break;
	case 1:
		(*matrices)[1].texIndex = 0.f;
		break;
	}

	glm::mat4 modelMatrix2(1.0f);
	(*matrices)[2].model = glm::translate(modelMatrix2, { -1.f,1.f,0.f });

	glm::mat4 modelMatrix3(1.0f);
	//(*matrices)[3].model = glm::translate(modelMatrix3, { -1.f,-1.f,0.f });
	(*matrices)[3].model = glm::scale(modelMatrix3, { 0.5f, 0.5f, 0.f });

	glm::mat4 modelMatrix4(1.0f);
	(*matrices)[4].model = glm::translate(modelMatrix4, { 1.f,-1.f,0.f });
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

