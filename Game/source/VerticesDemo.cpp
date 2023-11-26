#include "VerticesDemo.hpp"
#include "Engine.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

void VerticesDemo::Init()
{
	//Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	for (int i = 0; i < 3; ++i)
		Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
	Engine::Engine().GetVKRenderManager()->LoadQuad(glm::vec4(0.f, 0.f, 1.f, 1.f));
	Engine::Engine().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
}

void VerticesDemo::Update(float /*dt*/)
{
	auto matrices = Engine::Engine().GetVKRenderManager()->GetMatrices();
	//(*matrices)[0].model = glm::mat4(1);
	glm::mat4 modelMatrix(1.0f);
	(*matrices)[1].model = glm::translate(modelMatrix, { 1.f,1.f,0.f });
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

