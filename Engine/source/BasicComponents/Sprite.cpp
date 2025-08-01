//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: Sprtie.cpp
#include "BasicComponents/Sprite.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#pragma warning(pop)

#include <fstream>

#include "Engine.hpp"

Sprite::~Sprite()
{
	for (Animation* anim : animations)
	{
		delete anim;
	}
	animations.clear();
	DeleteFromSpriteManagerList();
}

void Sprite::Init()
{
}

void Sprite::Update(float dt)
{
	UpdateProjection();
	UpdateView();
	UpdateModel(GetOwner()->GetPosition(), GetOwner()->GetSize(), GetOwner()->GetRotate3D());

	UpdateAnimation(dt);
}

void Sprite::End()
{
	Engine::GetSpriteManager().DeleteSprite(this);
}

// @TODO Replace with bufferWrapper's Getter functions
void Sprite::UpdateModel(glm::vec3 pos_, glm::vec3 size_, float angle)
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
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
			break;
		case SpriteDrawType::ThreeDimension:
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.model = modelMatrix;
			// @TODO move to push constants later
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
			break;
		case SpriteDrawType::UI:
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
			break;
		}
	}
}

void Sprite::UpdateModel(glm::vec3 pos_, glm::vec3 size_, glm::vec3 angle)
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
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
			break;
		case SpriteDrawType::ThreeDimension:
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.model = modelMatrix;
			// @TODO move to push constants later
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
			break;
		case SpriteDrawType::UI:
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
			break;
		}
	}
}

void Sprite::UpdateView()
{
	for (auto& subMesh : subMeshes)
	{
		switch (spriteDrawType)
		{
		case SpriteDrawType::TwoDimension:
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.view = Engine::GetCameraManager().GetViewMatrix();
			break;
		case SpriteDrawType::ThreeDimension:
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.view = Engine::GetCameraManager().GetViewMatrix();
			// @TODO move to push constants later
			glm::mat4 inverseView = glm::inverse(subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.view);
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.viewPosition = glm::vec3(
				inverseView[3].x,
				inverseView[3].y,
				inverseView[3].z
			);
			break;
		case SpriteDrawType::UI:
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.view = glm::mat4(1.0f);
			break;
		}
	}
}

void Sprite::UpdateProjection()
{
	for (auto& subMesh : subMeshes)
	{
		switch (spriteDrawType)
		{
		case SpriteDrawType::TwoDimension:
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.projection = Engine::GetCameraManager().GetProjectionMatrix();
			break;
		case SpriteDrawType::ThreeDimension:
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.projection = Engine::GetCameraManager().GetProjectionMatrix();
			break;
		case SpriteDrawType::UI:
			glm::vec2 cameraViewSize = Engine::GetCameraManager().GetViewSize();
			subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.projection = glm::ortho(-cameraViewSize.x, cameraViewSize.x, -cameraViewSize.y, cameraViewSize.y, -1.f, 1.f);
			// Flip y-axis for Vulkan
			if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::VK)
			{
				subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.projection[1][1] *= -1;
			}
			break;
		}
	}
}

void Sprite::AddQuad(glm::vec4 color_)
{
	SubMesh subMesh;

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	subMesh.bufferWrapper.Initialize(Engine::GetRenderManager()->GetGraphicsMode(), RenderType::TwoDimension);

	auto& vertices = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;

	auto& indices = subMesh.bufferWrapper.GetIndices();
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	auto& vertexUniform = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.decode = renderManager->CreateMesh(vertices);
	vertexUniform.color = color_;
	vertexUniform.isTex = 0.f;
	vertexUniform.isTexel = 0.f;

	auto& fragmentUniform = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;
	fragmentUniform.texIndex = 0;

	renderManager->InitializeBuffers(subMesh.bufferWrapper, indices);
	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::UInt;
		position_layout.component_dimension = GLAttributeLayout::_1;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(TwoDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray->AddVertexBuffer(std::move(*subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray->SetIndexBuffer(std::move(*subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));
	}

	subMeshes.push_back(std::move(subMesh));

	SetSpriteDrawType(SpriteDrawType::TwoDimension);
	AddSpriteToManager();
}

void Sprite::AddQuadWithTexture(std::string name_, glm::vec4 color_, bool isTexel_)
{
	SubMesh subMesh;

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	subMesh.bufferWrapper.Initialize(Engine::GetRenderManager()->GetGraphicsMode(), RenderType::TwoDimension);

	auto& vertices = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;

	auto& indices = subMesh.bufferWrapper.GetIndices();
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	auto& vertexUniform = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.decode = renderManager->CreateMesh(vertices);
	vertexUniform.color = color_;
	vertexUniform.isTex = 0.f;
	vertexUniform.isTexel = isTexel_;

	auto& fragmentUniform = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;
	fragmentUniform.texIndex = 0;

	renderManager->InitializeBuffers(subMesh.bufferWrapper, indices);
	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::UInt;
		position_layout.component_dimension = GLAttributeLayout::_1;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(TwoDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray->AddVertexBuffer(std::move(*subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray->SetIndexBuffer(std::move(*subMesh.bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));
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

void Sprite::AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
{
	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
		[=](Sprite* sprite) { sprite->CreateMesh3D(type, path, stacks_, slices_, color, metallic_, roughness_); });
}

void Sprite::CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
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

void Sprite::DeleteFromSpriteManagerList()
{
	if (this != nullptr)
	{
		Engine::GetSpriteManager().DeleteSprite(this);
	}
}

void Sprite::LoadAnimation(const std::filesystem::path& spriteInfoFile, std::string name)
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
	AddQuadWithTexture(name, glm::vec4 (1.f), true);

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

glm::vec2 Sprite::GetHotSpot(int index)
{
	if (index < 0 || hotSpotList.size() <= index)
	{
		//Engine::GetLogger().LogError("Cannot find a hotspot of current index!");
		return glm::vec2{ 0,0 };
	}
	return hotSpotList[index];
}

void Sprite::PlayAnimation(int anim)
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

void Sprite::UpdateAnimation(float dt)
{
	if (animations.empty() == false && currAnim >= 0 && !animations[currAnim]->IsAnimationDone())
	{
		animations[currAnim]->Update(dt);
		auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
		vertexUniform.frameSize = glm::vec3(GetFrameSize() / textureSize, 0.f);
		vertexUniform.texelPos = glm::vec3(GetFrameTexel(animations[currAnim]->GetDisplayFrame()) / textureSize, 0.f);
	}
}

void Sprite::ChangeTexture(std::string name)
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
				auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
				auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerGL->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

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
				auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
				auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerVK->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

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
				auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
				auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerDX->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerDX->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerDX->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

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

void Sprite::AddSpriteToManager()
{
	Engine::GetSpriteManager().AddSprite(this);
}

void Sprite::SetColor(glm::vec4 color)
{
	if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

		vertexUniform.color = color;
	}
	else
	{
		auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;

		vertexUniform.color = color;
	}
}

glm::vec4 Sprite::GetColor()
{
	if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

		return vertexUniform.color;
	}
	else
	{
		auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;

		return vertexUniform.color;
	}
}

void Sprite::SetIsTex(bool state)
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
			auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	case GraphicsMode::DX:
	{
		//DXRenderManager* renderManagerDX = dynamic_cast<DXRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	case GraphicsMode::VK:
	{
		//VKRenderManager* renderManagerVK = dynamic_cast<VKRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			auto& vertexUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = subMeshes[0].bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	default:
		break;
	}
}

glm::vec2 Sprite::GetFrameTexel(int frameNum) const
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
