//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: ISprite.cpp
#include "Interface/ISprite.hpp"

#include "Engine.hpp"
#include "GLRenderManager.hpp"
#include "VKRenderManager.hpp"
#include "DXRenderManager.hpp"

void ISprite::AddQuad(glm::vec4 color_)
{
	Engine::GetObjectManager().QueueComponentFunction<ISprite>(this,
		[=](ISprite* sprite)
	{
		this->CreateQuad(color_);
	});
}

void ISprite::AddQuadWithTexture(std::string name_, glm::vec4 color_, bool isTexel_)
{
	Engine::GetObjectManager().QueueComponentFunction<ISprite>(this,
		[=](ISprite* sprite)
	{
		this->CreateQuadWithTexture(name_, color_, isTexel_);
	});
}

void ISprite::LoadAnimation(const std::filesystem::path& spriteInfoFile, std::string name)
{
	Engine::GetObjectManager().QueueComponentFunction<ISprite>(this,
		[=](ISprite* sprite)
	{
		this->LoadAnimationData(spriteInfoFile, name);
	});
}

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
	if (subMeshes.empty()) return { 1.f, 1.f, 1.f, 1.f };

	if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;

		return vertexUniform.color;
	}
	else
	{
		auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->vertexUniform;

		return vertexUniform.color;
	}
}

void ISprite::SetColor(glm::vec4 color)
{
	if (subMeshes.empty())
	{
		Engine::GetObjectManager().QueueComponentFunction<ISprite>(this, [=](ISprite* sprite)
		{
			sprite->SetColor(color);
		});
		return;
	}

	if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;

		vertexUniform.color = color;
	}
	else
	{
		auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->vertexUniform;

		vertexUniform.color = color;
	}
}

void ISprite::ChangeTexture(std::string name)
{
	if (subMeshes.empty())
	{
		Engine::GetObjectManager().QueueComponentFunction<ISprite>(this, [=](ISprite* sprite)
		{
			sprite->ChangeTexture(name);
		});
		return;
	}

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
				auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
				auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->fragmentUniform;

				fragmentUniform.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerGL->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->fragmentUniform;

				fragmentUniform.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->fragmentUniform;

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
				auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
				auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->fragmentUniform;

				fragmentUniform.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerVK->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->fragmentUniform;

				fragmentUniform.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->fragmentUniform;

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
				auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
				auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->fragmentUniform;

				fragmentUniform.texIndex = renderManagerDX->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerDX->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->fragmentUniform;

				fragmentUniform.texIndex = renderManagerDX->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->fragmentUniform;

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
	if (subMeshes.empty())
	{
		Engine::GetObjectManager().QueueComponentFunction<ISprite>(this, [=](ISprite* sprite)
		{
			sprite->SetIsTex(state);
		});
		return;
	}

	isTex = state;
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	switch (renderManager->GetGraphicsMode())
	{
	case GraphicsMode::GL:
	{
		//GLRenderManager* renderManagerGL = dynamic_cast<GLRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	case GraphicsMode::DX:
	{
		//DXRenderManager* renderManagerDX = dynamic_cast<DXRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	case GraphicsMode::VK:
	{
		//VKRenderManager* renderManagerVK = dynamic_cast<VKRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	default:
		break;
	}
}

glm::vec3 ISprite::GetSpecularColor(int index)
{
	if (index < 0 || index >= static_cast<int>(subMeshes.size())) return { 0.f, 0.f, 0.f };
	return subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.specularColor;
}

float ISprite::GetShininess(int index)
{
	if (index < 0 || index >= static_cast<int>(subMeshes.size())) return 0.f;
	return subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.shininess;
}

float ISprite::GetMetallic(int index)
{
	if (index < 0 || index >= static_cast<int>(subMeshes.size())) return 0.f;
	return subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.metallic;
}

float ISprite::GetRoughness(int index)
{
	if (index < 0 || index >= static_cast<int>(subMeshes.size())) return 0.f;
	return subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.roughness;
}

void ISprite::SetSpecularColor(glm::vec3 sColor, int index)
{
	if (index < 0 || index >= static_cast<int>(subMeshes.size()))
	{
		Engine::GetObjectManager().QueueComponentFunction<ISprite>(this, [=](ISprite* sprite)
		{
			sprite->SetSpecularColor(sColor, index);
		});
		return;
	}
	subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.specularColor = sColor;
}

void ISprite::SetShininess(float amount, int index)
{
	if (index < 0 || index >= static_cast<int>(subMeshes.size()))
	{
		Engine::GetObjectManager().QueueComponentFunction<ISprite>(this, [=](ISprite* sprite)
		{
			sprite->SetShininess(amount, index);
		});
		return;
	}
	subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.shininess = amount;
}

void ISprite::SetMetallic(float amount, int index)
{
	if (index < 0 || index >= static_cast<int>(subMeshes.size()))
	{
		Engine::GetObjectManager().QueueComponentFunction<ISprite>(this, [=](ISprite* sprite)
		{
			sprite->SetMetallic(amount, index);
		});
		return;
	}
	subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.metallic = amount;
}

void ISprite::SetRoughness(float amount, int index)
{
	if (index < 0 || index >= static_cast<int>(subMeshes.size()))
	{
		Engine::GetObjectManager().QueueComponentFunction<ISprite>(this, [=](ISprite* sprite)
		{
			sprite->SetRoughness(amount, index);
		});
		return;
	}
	subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.roughness = amount;
}
void ISprite::SetSpriteDrawType(SpriteDrawType type)
{
	if (subMeshes.empty())
	{
		Engine::GetObjectManager().QueueComponentFunction<ISprite>(this, [=](ISprite* sprite)
		{
			sprite->SetSpriteDrawType(type);
		});
		return;
	}

	spriteDrawType = type;
}
