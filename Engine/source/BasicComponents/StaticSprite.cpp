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

#include <fstream>

#include "Engine.hpp"

StaticSprite::~StaticSprite()
{
	RenderManager* renderManager = Engine::GetRenderManager();
	// @TODO Make OpenGL, Vulkan version of SafeDelete, ProcessDeletionQueue function and remove ProcessFunctionQueue(), DeleteObjectsFromList()
	if (renderManager->GetGraphicsMode() == GraphicsMode::DX)
	{
		for (auto& subMesh : subMeshes)
		{
			dynamic_cast<DXRenderManager*>(renderManager)->SafeDelete(std::move(subMesh));
		}
	}
	//DeleteFromSpriteManagerList();
	Engine::GetLogger().LogDebug(LogCategory::Object, "Component Deleted : Static Sprite");
}

void StaticSprite::Init()
{
	Engine::GetLogger().LogDebug(LogCategory::Object, "Component Added : Static Sprite");
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
	case GraphicsMode::GL:
		if (spriteDrawType == SpriteDrawType::ThreeDimension)
		{
			pos = glm::vec3(pos_.x, pos_.y, pos_.z);
		}
		else
		{
			pos = glm::vec3(pos_.x * 2, pos_.y * 2, pos_.z);
		}
		modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
			glm::rotate(glm::mat4(1.0f), glm::radians(-angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		break;
	case GraphicsMode::VK:
		if (spriteDrawType == SpriteDrawType::ThreeDimension)
		{
			pos = glm::vec3(pos_.x, pos_.y, pos_.z);

			modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
				glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		}
		else
		{
			pos = glm::vec3(pos_.x * 2, pos_.y * 2, pos_.z);

			modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
				glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		}
		break;
	case GraphicsMode::DX:
		if (spriteDrawType == SpriteDrawType::ThreeDimension)
		{
			pos = glm::vec3(pos_.x, pos_.y, pos_.z);

			modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
				glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		}
		else
		{
			pos = glm::vec3(pos_.x * 2, pos_.y * 2, pos_.z);

			modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
				glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		}
		break;
	}

	for (auto& subMesh : subMeshes)
	{
		switch (spriteDrawType)
		{
		case SpriteDrawType::TwoDimension:
			subMesh->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
			break;
		case SpriteDrawType::ThreeDimension:
			subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.model = modelMatrix;
			// @TODO move to push constants later
			subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
			break;
		case SpriteDrawType::UI:
			subMesh->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
			break;
		}
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
		if (spriteDrawType == SpriteDrawType::ThreeDimension)
		{
			rotationMatrix = glm::toMat4(glm::quat(glm::radians(-angle)));
			pos = glm::vec3(pos_.x, pos_.y, pos_.z);
		}
		else
		{
			rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-angle.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(-angle.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(-angle.z), glm::vec3(0.0f, 0.0f, 1.0f));
			pos = glm::vec3(pos_.x * 2, pos_.y * 2, pos_.z);
		}
		modelMatrix = glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *
			glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		break;
	case GraphicsMode::VK:
		if (spriteDrawType == SpriteDrawType::ThreeDimension)
		{
			rotationMatrix = glm::toMat4(glm::quat(glm::radians(glm::vec3(-angle.x, -angle.y, -angle.z))));
			pos = glm::vec3(pos_.x, pos_.y, pos_.z);

			modelMatrix = glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *
				glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		}
		else
		{
			rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-angle.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(-angle.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(-angle.z), glm::vec3(0.0f, 0.0f, 1.0f));
			pos = glm::vec3(pos_.x * 2, pos_.y * 2, pos_.z);

			modelMatrix = glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *
				glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		}
		break;
	case GraphicsMode::DX:
		if (spriteDrawType == SpriteDrawType::ThreeDimension)
		{
			rotationMatrix = glm::toMat4(glm::quat(glm::radians(glm::vec3(-angle.x, -angle.y, -angle.z))));
			pos = glm::vec3(pos_.x, pos_.y, pos_.z);

			modelMatrix = glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *
				glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		}
		else
		{
			rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-angle.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(-angle.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(-angle.z), glm::vec3(0.0f, 0.0f, 1.0f));
			pos = glm::vec3(pos_.x * 2, pos_.y * 2, pos_.z);

			modelMatrix = glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *
				glm::scale(glm::mat4(1.0f), glm::vec3(size_.x, size_.y, size_.z));
		}
		break;
	}

	for (auto& subMesh : subMeshes)
	{
		switch (spriteDrawType)
		{
		case SpriteDrawType::TwoDimension:
			subMesh->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
			break;
		case SpriteDrawType::ThreeDimension:
			subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.model = modelMatrix;
			// @TODO move to push constants later
			subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
			break;
		case SpriteDrawType::UI:
			subMesh->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
			break;
		}
	}
}

void StaticSprite::UpdateView()
{
	for (auto& subMesh : subMeshes)
	{
		switch (spriteDrawType)
		{
		case SpriteDrawType::TwoDimension:
			subMesh->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.view = Engine::GetCameraManager().GetViewMatrix();
			break;
		case SpriteDrawType::ThreeDimension:
			subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.view = Engine::GetCameraManager().GetViewMatrix();
			// @TODO move to push constants later
			glm::mat4 inverseView = glm::inverse(subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.view);
			subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.viewPosition = glm::vec3(
				inverseView[3].x,
				inverseView[3].y,
				inverseView[3].z
			);
			break;
		case SpriteDrawType::UI:
			subMesh->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.view = glm::mat4(1.0f);
			break;
		}
	}
}

void StaticSprite::UpdateProjection()
{
	for (auto& subMesh : subMeshes)
	{
		switch (spriteDrawType)
		{
		case SpriteDrawType::TwoDimension:
			subMesh->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.projection = Engine::GetCameraManager().GetProjectionMatrix();
			break;
		case SpriteDrawType::ThreeDimension:
			subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.projection = Engine::GetCameraManager().GetProjectionMatrix();
			break;
		case SpriteDrawType::UI:
			glm::vec2 cameraViewSize = Engine::GetCameraManager().GetViewSize();
			subMesh->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.projection = glm::ortho(-cameraViewSize.x, cameraViewSize.x, -cameraViewSize.y, cameraViewSize.y, -1.f, 1.f);
			// Flip y-axis for Vulkan
			if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::VK)
			{
				subMesh->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.projection[1][1] *= -1;
			}
			break;
		}
	}
}

void StaticSprite::CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
{
	meshType = type;
	filePath = path;
	stacks = stacks_;
	slices = slices_;

	auto& objectMap = Engine::GetObjectManager().GetObjectMap();
	for (const auto& object : objectMap)
	{
		if (object.second.get()->HasComponent<StaticSprite>())
		{
			m_meshIndex++;
		}
	}

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->CreateStaticMesh(subMeshes, type, path, stacks, slices, color, metallic_, roughness_);

	SetSpriteDrawType(SpriteDrawType::ThreeDimension);
	//AddSpriteToManager();
}

// This must be called after loading whole mesh data into BufferWrapper
void StaticSprite::InitializeBuffers()
{
	Engine::GetSpriteManager().RegisterStaticSprite();
}
