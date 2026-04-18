//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: DynamicSprite.cpp
#include "BasicComponents/DynamicSprite.hpp"
#include "BasicComponents/SkeletalAnimator.hpp"
#include "Animation.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#pragma warning(pop)

#include <fstream>

#include "Engine.hpp"
#include "GLRenderManager.hpp"
#include "VKRenderManager.hpp"
#include "DXRenderManager.hpp"

DynamicSprite::~DynamicSprite()
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
	for (Animation* anim : animations)
	{
		delete anim;
	}
	animations.clear();
	DeleteFromSpriteManagerList();
}

void DynamicSprite::Init()
{
}

void DynamicSprite::Update(float dt)
{
	UpdateProjection();
	UpdateView();
	
	
	// object transform 
	UpdateModel(GetOwner()->GetPosition(), GetOwner()->GetSize(), GetOwner()->GetRotate3D());

	UpdateAnimation(dt);
}

void DynamicSprite::End()
{
	Engine::GetSpriteManager().DeleteDynamicSprite(this);
}

void DynamicSprite::AddSpriteToManager()
{
	Engine::GetSpriteManager().AddDynamicSprite(this);
}

void DynamicSprite::DeleteFromSpriteManagerList()
{
	if (this != nullptr)
	{
		Engine::GetSpriteManager().DeleteDynamicSprite(this);
	}
}

// @TODO Replace with bufferWrapper's Getter functions
void DynamicSprite::UpdateModel(glm::vec3 pos_, glm::vec3 size_, float angle)
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
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
			vertexUniform.model = modelMatrix;
			break;
		}
		case SpriteDrawType::ThreeDimension:
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>()->vertexUniform;
			vertexUniform.model = modelMatrix;
			// @TODO move to push constants later
			vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
			break;
		}
		case SpriteDrawType::UI:
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
			vertexUniform.model = modelMatrix;
			break;
		}
		}
	}
}

void DynamicSprite::UpdateModel(glm::vec3 pos_, glm::vec3 size_, glm::vec3 angle)
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
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
			vertexUniform.model = modelMatrix;
			break;
		}
		case SpriteDrawType::ThreeDimension:
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>()->vertexUniform;
			vertexUniform.model = modelMatrix;
			// @TODO move to push constants later
			vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
			break;
		}
		case SpriteDrawType::UI:
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
			vertexUniform.model = modelMatrix;
			break;
		}
		}
	}
}

void DynamicSprite::UpdateView()
{
	for (auto& subMesh : subMeshes)
	{
		switch (spriteDrawType)
		{
		case SpriteDrawType::TwoDimension:
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
			vertexUniform.view = Engine::GetCameraManager().GetViewMatrix();
			break;
		}
		case SpriteDrawType::ThreeDimension:
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
			break;
		}
		case SpriteDrawType::UI:
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
			vertexUniform.view = glm::mat4(1.0f);
			break;
		}
		}
	}
}

void DynamicSprite::UpdateProjection()
{
	for (auto& subMesh : subMeshes)
	{
		switch (spriteDrawType)
		{
		case SpriteDrawType::TwoDimension:
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
			vertexUniform.projection = Engine::GetCameraManager().GetProjectionMatrix();
			break;
		}
		case SpriteDrawType::ThreeDimension:
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>()->vertexUniform;
			vertexUniform.projection = Engine::GetCameraManager().GetProjectionMatrix();
			break;
		}
		case SpriteDrawType::UI:
		{
			auto& vertexUniform = subMesh->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
			glm::vec2 cameraViewSize = Engine::GetCameraManager().GetViewSize();
			vertexUniform.projection = glm::ortho(-cameraViewSize.x, cameraViewSize.x, -cameraViewSize.y, cameraViewSize.y, -1.f, 1.f);
			// Flip y-axis for Vulkan
			if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::VK)
			{
				vertexUniform.projection[1][1] *= -1;
			}
			break;
		}
		}
	}
}

void DynamicSprite::CreateQuad(glm::vec4 color_)
{
	SubMesh subMesh;

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	subMesh = std::make_unique<BufferWrapper>(SpriteType::DYNAMIC);

	auto* sprite = subMesh->GetData<BufferWrapper::DynamicSprite2D>();

	auto& vertices = sprite->vertices;

	auto& indices = sprite->indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	auto& vertexUniform = sprite->vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.decode = renderManager->CreateMesh(vertices);
	vertexUniform.color = color_;
	vertexUniform.isTex = 0.f;
	vertexUniform.isTexel = 0.f;

	auto& fragmentUniform = sprite->fragmentUniform;
	fragmentUniform.texIndex = 0;

	renderManager->InitializeDynamicBuffers(*subMesh, indices);
	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		auto* buffer = subMesh->GetBuffer<BufferWrapper::GLBuffer>();
		buffer->vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::UInt;
		position_layout.component_dimension = GLAttributeLayout::_1;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(TwoDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		buffer->vertexArray->AddVertexBuffer(std::move(*buffer->vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		buffer->vertexArray->SetIndexBuffer(std::move(*buffer->indexBuffer));
	}

	subMeshes.push_back(std::move(subMesh));

	SetSpriteDrawType(SpriteDrawType::TwoDimension);
	AddSpriteToManager();
}

void DynamicSprite::CreateQuadWithTexture(std::string name_, glm::vec4 color_, bool isTexel_)
{
	SubMesh subMesh;

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	subMesh = std::make_unique<BufferWrapper>(SpriteType::DYNAMIC);

	auto* sprite = subMesh->GetData<BufferWrapper::DynamicSprite2D>();

	auto& vertices = sprite->vertices;

	auto& indices = sprite->indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	auto& vertexUniform = sprite->vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.decode = renderManager->CreateMesh(vertices);
	vertexUniform.color = color_;
	vertexUniform.isTex = 0.f;
	vertexUniform.isTexel = isTexel_;

	auto& fragmentUniform = sprite->fragmentUniform;
	fragmentUniform.texIndex = 0;

	renderManager->InitializeDynamicBuffers(*subMesh, indices);
	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		auto* buffer = subMesh->GetBuffer<BufferWrapper::GLBuffer>();
		buffer->vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::UInt;
		position_layout.component_dimension = GLAttributeLayout::_1;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(TwoDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		buffer->vertexArray->AddVertexBuffer(std::move(*buffer->vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		buffer->vertexArray->SetIndexBuffer(std::move(*buffer->indexBuffer));
	}

	subMeshes.push_back(std::move(subMesh));

	ChangeTexture(name_);
	switch (renderManager->GetGraphicsMode())
	{
	case GraphicsMode::GL:
		textureSize = dynamic_cast<GLRenderManager*>(renderManager)->GetTexture(name_)->GetSize();
		break;
	case GraphicsMode::VK:
		textureSize = dynamic_cast<VKRenderManager*>(renderManager)->GetTexture(name_)->GetSize();
		break;
	case GraphicsMode::DX:
		textureSize = dynamic_cast<DXRenderManager*>(renderManager)->GetTexture(name_)->GetSize();
		break;
	}

	SetSpriteDrawType(SpriteDrawType::TwoDimension);
	AddSpriteToManager();
}

void DynamicSprite::CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
{
	meshType = type;
	filePath = path;
	stacks = stacks_;
	slices = slices_;

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->CreateMesh(subMeshes, type, path, stacks, slices, color, metallic_, roughness_);

	SetSpriteDrawType(SpriteDrawType::ThreeDimension);
	AddSpriteToManager();
}

void DynamicSprite::LoadAnimationData(const std::filesystem::path& spriteInfoFile, std::string name)
{
	hotSpotList.clear();
	frameTexel.clear();
	animations.clear();

	if (spriteInfoFile.extension() != ".spt")
	{
		throw std::runtime_error("Bad Filetype.  " + spriteInfoFile.generic_string() + " not a sprite info file (.spt)");
	}
	std::ifstream inFile(spriteInfoFile);

	if (inFile.is_open() == false)
	{
		throw std::runtime_error("Failed to load " + spriteInfoFile.generic_string());
	}

	std::string text;
	inFile >> text;
	//texturePtr = Engine::GetTextureManager().Load(text, true);
	//frameSize = texturePtr->GetSize();
	Engine::Instance().GetRenderManager()->LoadTexture(text, name, true);
	CreateQuadWithTexture(name, glm::vec4(1.f), true);

	inFile >> text;
	while (inFile.eof() == false)
	{
		if (text == "FrameSize")
		{
			inFile >> frameSize.x;
			inFile >> frameSize.y;
		}
		else if (text == "NumFrames")
		{
			int numFrames;
			inFile >> numFrames;
			for (int i = 0; i < numFrames; i++)
			{
				frameTexel.push_back({ frameSize.x * i, 0 });
			}
		}
		else if (text == "Frame")
		{
			int frameLocationX, frameLocationY;
			inFile >> frameLocationX;
			inFile >> frameLocationY;
			frameTexel.push_back({ static_cast<float>(frameLocationX), static_cast<float>(frameLocationY) });
		}
		else if (text == "HotSpot")
		{
			int hotSpotX, hotSpotY;
			inFile >> hotSpotX;
			inFile >> hotSpotY;
			hotSpotList.push_back({ static_cast<float>(hotSpotX), static_cast<float>(hotSpotY) });
		}
		else if (text == "Anim")
		{
			inFile >> text;
			animations.push_back(new Animation{ text });
		}
		//else if (text == "CollisionRect")
		//{
		//	rect3 rect;
		//	inFile >> rect.point1.x >> rect.point1.y >> rect.point2.x >> rect.point2.y;
		//	if (object == nullptr)
		//	{
		//		Engine::GetLogger().LogError("Trying to add collision to a nullobject");
		//	}
		//	else
		//	{
		//		object->AddGOComponent(new RectCollision(rect, object));
		//	}
		//}
		//else if (text == "CollisionCircle")
		//{
		//	double radius;
		//	inFile >> radius;
		//	if (object == nullptr)
		//	{
		//		Engine::GetLogger().LogError("Trying to add collision to a nullobject");
		//	}
		//	else
		//	{
		//		object->AddGOComponent(new CircleCollision(radius, object));
		//	}
		//}
		else
		{
			//Engine::GetLogger().LogError("Unknown spt command " + text);
		}
		inFile >> text;
	}

	if (frameTexel.empty() == true)
	{
		frameTexel.push_back({ 0,0 });
	}

	if (animations.empty())
	{
		animations.push_back(new Animation{});
		PlayAnimation(0);
	}
}

glm::vec2 DynamicSprite::GetHotSpot(int index)
{
	if (index < 0 || hotSpotList.size() <= index)
	{
		//Engine::GetLogger().LogError("Cannot find a hotspot of current index!");
		return glm::vec2{ 0,0 };
	}
	return hotSpotList[index];
}

void DynamicSprite::PlayAnimation(int anim)
{
	if (anim < 0 || animations.size() <= anim)
	{
		//Engine::GetLogger().LogError(std::to_string(anim) + " is out of index!");
		currAnim = 0;
	}
	else
	{
		currAnim = anim;
		animations[currAnim]->ResetAnimation();
	}
}

bool DynamicSprite::IsAnimationDone() const
{
	return animations[currAnim]->IsAnimationDone();
}

void DynamicSprite::UpdateAnimation(float dt) const
{
	if (animations.empty() == false && currAnim >= 0 && !animations[currAnim]->IsAnimationDone())
	{
		animations[currAnim]->Update(dt);
		auto& vertexUniform = subMeshes[0]->GetData<BufferWrapper::DynamicSprite2D>()->vertexUniform;
		vertexUniform.frameSize = glm::vec3(GetFrameSize() / textureSize, 0.f);
		vertexUniform.texelPos = glm::vec3(GetFrameTexel(animations[currAnim]->GetDisplayFrame()) / textureSize, 0.f);
	}
}

glm::vec2 DynamicSprite::GetFrameTexel(int frameNum) const
{
	if (frameNum < 0 || frameTexel.size() <= frameNum)
	{
		//Engine::GetLogger().LogError(std::to_string(frameNum) + " is out of index!");
		return glm::vec2{ 0,0 };
	}
	else
	{
		return frameTexel[frameNum];
	}
}