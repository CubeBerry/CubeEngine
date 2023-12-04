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

	glm::vec3 pos(0, 0, 0);
	glm::vec3 size(Engine::GetWindow()->GetWindowSize(), 0);
	glm::vec3 extent(1.f / Engine::GetWindow()->GetWindowSize().x, 1.f / Engine::GetWindow()->GetWindowSize().y, 0);

	modelMatrix = glm::translate(modelMatrix, GetOwner()->GetPosition() * extent);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(GetOwner()->GetRotate()), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, GetOwner()->GetSize() * extent);

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

void MaterialComponent::AddMeshWithVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_)
{
	Engine::Instance().GetVKRenderManager()->LoadVertices(vertices_, indices_);
	materialId = Engine::Instance().GetVKRenderManager()->GetMatrices()->size() - 1;
}

void MaterialComponent::AddMeshWithLineVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_)
{
	Engine::Instance().GetVKRenderManager()->LoadLineVertices(vertices_, indices_);
	materialId = Engine::Instance().GetVKRenderManager()->GetMatrices()->size() - 1;
}

void MaterialComponent::AddMeshWithTexture(float index)
{
	Engine::Instance().GetVKRenderManager()->LoadQuad({1.f,1.f,1.f,1.f}, true);
	materialId = Engine::Instance().GetVKRenderManager()->GetMatrices()->size() - 1;
	ChangeTexture(index);
}

void MaterialComponent::ChangeTexture(float index)
{
	textureId = static_cast<int>(index);
	Engine::Instance().GetVKRenderManager()->GetMatrices()->at(materialId).texIndex = index;
}
