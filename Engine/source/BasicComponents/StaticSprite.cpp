//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: StaticSprite.cpp
#include "BasicComponents/StaticSprite.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#pragma warning(pop)

#include "Engine.hpp"
#include "DXRenderManager.hpp"

StaticSprite::~StaticSprite()
{
	//RenderManager* renderManager = Engine::GetRenderManager();
	//// @TODO Make OpenGL, Vulkan version of SafeDelete, ProcessDeletionQueue function and remove ProcessFunctionQueue(), DeleteObjectsFromList()
	//if (renderManager->GetGraphicsMode() == GraphicsMode::DX)
	//{
	//	for (auto& subMesh : subMeshes)
	//	{
	//		dynamic_cast<DXRenderManager*>(renderManager)->SafeDelete(std::move(subMesh));
	//	}
	//}
	//DeleteFromSpriteManagerList();
}

void StaticSprite::Init()
{
}

void StaticSprite::Update(float dt)
{
	UpdateProjection();
	UpdateView();
	UpdateModel(GetOwner()->GetPosition(), GetOwner()->GetSize(), GetOwner()->GetRotate3D());
}

void StaticSprite::End()
{
	//Engine::GetSpriteManager().DeleteSprite(this);
}

// @TODO Replace with bufferWrapper's Getter functions
void StaticSprite::UpdateModel(glm::vec3 pos_, glm::vec3 size_, float angle)
{
	glm::mat4 modelMatrix(1.0f);
	glm::vec3 pos;

	switch (Engine::GetRenderManager()->GetGraphicsMode())
	{
		pos = glm::vec3(pos_.x, pos_.y, pos_.z);
		modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
			glm::rotate(glm::mat4(1.0f), glm::radians(-angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		break;
	case GraphicsMode::VK:
		pos = glm::vec3(pos_.x, pos_.y, pos_.z);

		modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
			glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		break;
	case GraphicsMode::DX:
		pos = glm::vec3(pos_.x, pos_.y, pos_.z);

		modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
			glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		break;
	}

	for (auto& subMesh : subMeshes)
	{
		auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>()->vertexUniform;
		vertexUniform.model = modelMatrix;
		// @TODO move to push constants later
		vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
	}
}

void StaticSprite::UpdateModel(glm::vec3 pos_, glm::vec3 size_, glm::vec3 angle)
{
	glm::mat4 modelMatrix(1.0f);
	glm::mat4 rotationMatrix(1.0f);
	glm::vec3 pos;

	switch (Engine::GetRenderManager()->GetGraphicsMode())
	{
	case GraphicsMode::GL:
		rotationMatrix = glm::toMat4(glm::quat(glm::radians(-angle)));
		pos = glm::vec3(pos_.x, pos_.y, pos_.z);
		modelMatrix = glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *
			glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		break;
	case GraphicsMode::VK:
		rotationMatrix = glm::toMat4(glm::quat(glm::radians(glm::vec3(-angle.x, -angle.y, -angle.z))));
		pos = glm::vec3(pos_.x, pos_.y, pos_.z);

		modelMatrix = glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *
			glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		break;
	case GraphicsMode::DX:
		rotationMatrix = glm::toMat4(glm::quat(glm::radians(glm::vec3(-angle.x, -angle.y, -angle.z))));
		pos = glm::vec3(pos_.x, pos_.y, pos_.z);

		modelMatrix = glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *
			glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		break;
	}

	for (auto& subMesh : subMeshes)
	{
		auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>()->vertexUniform;
		vertexUniform.model = modelMatrix;
		// @TODO move to push constants later
		vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
		break;
	}
}

void StaticSprite::UpdateView()
{
	for (auto& subMesh : subMeshes)
	{
		auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>()->vertexUniform;
		vertexUniform.view = Engine::GetCameraManager().GetViewMatrix();
		// @TODO move to push constants later
		glm::mat4 inverseView = glm::inverse(vertexUniform.view);
		vertexUniform.viewPosition = glm::vec4(
			inverseView[3].x,
			inverseView[3].y,
			inverseView[3].z,
			1.0f
		);
	}
}

void StaticSprite::UpdateProjection()
{
	for (auto& subMesh : subMeshes)
	{
		auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>()->vertexUniform;
		vertexUniform.projection = Engine::GetCameraManager().GetProjectionMatrix();
	}
}

void StaticSprite::CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
{
	meshType = type;
	filePath = path;
	stacks = stacks_;
	slices = slices_;

	//auto& objectMap = Engine::GetObjectManager().GetObjectMap();
	//for (const auto& object : objectMap)
	//{
	//	if (object.second.get()->HasComponent<StaticSprite>())
	//	{
	//		m_meshIndex++;
	//	}
	//}

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->CreateMesh(subMeshes, type, path, stacks, slices, color, metallic_, roughness_);

	SetSpriteDrawType(SpriteDrawType::ThreeDimension);
	//AddSpriteToManager();
}

// This must be called after loading whole mesh data into BufferWrapper
void StaticSprite::InitializeBuffers()
{
	Engine::GetSpriteManager().RegisterStaticSprite();
}
