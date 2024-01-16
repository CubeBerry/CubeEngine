#include "BasicComponents/MaterialComponent.hpp"
#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

MaterialComponent::~MaterialComponent()
{
}

void MaterialComponent::Init()
{
}

void MaterialComponent::Update(float /*dt*/)
{
	UpdateProjection();
	UpdateView();
	UpdateModel();
}

void MaterialComponent::End()
{
}

void MaterialComponent::UpdateModel()
{
	glm::mat4 modelMatrix(1.0f);
	glm::vec3 pos = glm::vec3(GetOwner()->GetPosition().x * 2, GetOwner()->GetPosition().y * 2, GetOwner()->GetPosition().z);
	modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
		glm::rotate(glm::mat4(1.0f), glm::radians(GetOwner()->GetRotate()), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(GetOwner()->GetSize().x, GetOwner()->GetSize().y, GetOwner()->GetSize().z));

	Engine::Instance().GetVKRenderManager()->GetMatrices()->at(materialId).model = modelMatrix;
}

void MaterialComponent::UpdateView()
{
	Engine::Instance().GetVKRenderManager()->GetMatrices()->at(materialId).view = Engine::Engine().GetCameraManager()->GetViewMatrix();
}

void MaterialComponent::UpdateProjection()
{
	Engine::Instance().GetVKRenderManager()->GetMatrices()->at(materialId).projection = Engine::Engine().GetCameraManager()->GetProjectionMatrix();
}

void MaterialComponent::AddQuad(glm::vec4 color_, float isTex_)
{
	Engine::Instance().GetVKRenderManager()->LoadQuad(color_, isTex_);
	materialId = Engine::Instance().GetVKRenderManager()->GetMatrices()->size() - 1;
}

void MaterialComponent::AddQuadLine(glm::vec4 color_)
{
	Engine::Instance().GetVKRenderManager()->LoadLineQuad(color_);
	materialId = Engine::Instance().GetVKRenderManager()->GetMatrices()->size() - 1;
}

void MaterialComponent::AddMeshWithTexture(int index)
{
	Engine::Instance().GetVKRenderManager()->LoadQuad({1.f,1.f,1.f,1.f}, 1.f);
	materialId = Engine::Instance().GetVKRenderManager()->GetMatrices()->size() - 1;
	ChangeTexture(index);
}

void MaterialComponent::ChangeTexture(int index)
{
	textureId = index;
	Engine::Instance().GetVKRenderManager()->GetMatrices()->at(materialId).texIndex = index;
}
