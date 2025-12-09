//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: ISprite.cpp
#include "BasicComponents/ISprite.hpp"

#include "Engine.hpp"

void ISprite::AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
{
	Engine::GetObjectManager().QueueComponentFunction<ISprite>(this,
		[=](ISprite* sprite)
		{
			this->CreateMesh3D(type, path, stacks_, slices_, color, metallic_, roughness_);
		});
}

glm::vec4 ISprite::GetColor()
{
	if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

		return vertexUniform.color;
	}
	else
	{
		auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;

		return vertexUniform.color;
	}
}

void ISprite::SetColor(glm::vec4 color)
{
	if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

		vertexUniform.color = color;
	}
	else
	{
		auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;

		vertexUniform.color = color;
	}
}

void ISprite::ChangeTexture(std::string name)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	switch (renderManager->GetGraphicsMode())
	{
	case GraphicsMode::GL:
	{
		GLRenderManager* renderManagerGL = dynamic_cast<GLRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			if (renderManagerGL->GetTexture(name) != nullptr)
			{
				auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
				auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerGL->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.isTex = false;
				isTex = false;
			}
		}
		break;
	}
	case GraphicsMode::VK:
	{
		VKRenderManager* renderManagerVK = dynamic_cast<VKRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			if (renderManagerVK->GetTexture(name) != nullptr)
			{
				auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
				auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerVK->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.isTex = false;
				isTex = false;
			}
		}
		break;
	}
	case GraphicsMode::DX:
	{
		DXRenderManager* renderManagerDX = dynamic_cast<DXRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			if (renderManagerDX->GetTexture(name) != nullptr)
			{
				auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
				auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerDX->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerDX->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerDX->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.isTex = false;
				isTex = false;
			}
		}
		break;
	}
	default:
		break;
	}
}

void ISprite::SetIsTex(bool state)
{
	isTex = state;
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	switch (renderManager->GetGraphicsMode())
	{
	case GraphicsMode::GL:
	{
		//GLRenderManager* renderManagerGL = dynamic_cast<GLRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	case GraphicsMode::DX:
	{
		//DXRenderManager* renderManagerDX = dynamic_cast<DXRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	case GraphicsMode::VK:
	{
		//VKRenderManager* renderManagerVK = dynamic_cast<VKRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			auto& vertexUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = subMeshes[0]->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	default:
		break;
	}
}
