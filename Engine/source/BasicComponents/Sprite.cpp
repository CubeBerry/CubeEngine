//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: Sprtie.cpp
#include "BasicComponents/Sprite.hpp"
#include "Engine.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#pragma warning(pop)

#include <fstream>

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
	}

	switch (spriteDrawType)
	{
	case SpriteDrawType::TwoDimension:
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).model = modelMatrix;
		break;
	case SpriteDrawType::ThreeDimension:
		Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at(materialId).model = modelMatrix;
		break;
	case SpriteDrawType::UI:
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).model = modelMatrix;
		break;
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
	}

	switch (spriteDrawType)
	{
	case SpriteDrawType::TwoDimension:
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).model = modelMatrix;
		break;
	case SpriteDrawType::ThreeDimension:
		Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at(materialId).model = modelMatrix;
		break;
	case SpriteDrawType::UI:
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).model = modelMatrix;
		break;
	}
}

void Sprite::UpdateView()
{
	switch (spriteDrawType)
	{
	case SpriteDrawType::TwoDimension:
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).view = Engine::GetCameraManager().GetViewMatrix();
		break;
	case SpriteDrawType::ThreeDimension:
		Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at(materialId).view = Engine::GetCameraManager().GetViewMatrix();
		break;
	case SpriteDrawType::UI:
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).view = glm::mat4(1.0f);
		break;
	}
}

void Sprite::UpdateProjection()
{
	switch (spriteDrawType)
	{
	case SpriteDrawType::TwoDimension:
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).projection = Engine::GetCameraManager().GetProjectionMatrix();
		break;
	case SpriteDrawType::ThreeDimension:
		Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at(materialId).projection = Engine::GetCameraManager().GetProjectionMatrix();
		break;
	case SpriteDrawType::UI:
		glm::vec2 cameraViewSize = Engine::GetCameraManager().GetViewSize();
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).projection = glm::ortho(-cameraViewSize.x, cameraViewSize.x, -cameraViewSize.y, cameraViewSize.y, -1.f, 1.f);
		if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::VK)
		{
			Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).projection[1][1] *= -1;
		}
		break;
	}
}


void Sprite::AddQuad(glm::vec4 color_)
{
	Engine::Instance().GetRenderManager()->LoadQuad(color_, 0.f, 0.f);
	materialId = static_cast<int>(Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->size() - 1);
	AddSpriteToManager();
}

//void Sprite::AddQuadLine(glm::vec4 color_)
//{
//	Engine::Instance().GetVKRenderManager().LoadLineQuad(color_);
//	materialId = static_cast<int>(Engine::Instance().GetVKRenderManager().GetVertexVector()->size() - 1);
//	AddSpriteToManager();
//}

void Sprite::AddMeshWithTexture(std::string name_, glm::vec4 color_)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->LoadQuad(color_, 1.f, 0.f);
	materialId = Engine::GetSpriteManager().GetSpritesAmount();
	ChangeTexture(name_);
	switch (renderManager->GetGraphicsMode())
	{
	case GraphicsMode::GL:
		textureSize = dynamic_cast<GLRenderManager*>(renderManager)->GetTexture(name_)->GetSize();
		break;
	case GraphicsMode::VK:
		textureSize = dynamic_cast<VKRenderManager*>(renderManager)->GetTexture(name_)->GetSize();
		break;
	}
	AddSpriteToManager();
}

void Sprite::AddMeshWithTexel(std::string name_, glm::vec4 color_)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->LoadQuad(color_, 1.f, 1.f);
	materialId = Engine::GetSpriteManager().GetSpritesAmount();
	ChangeTexture(name_);
	switch (renderManager->GetGraphicsMode())
	{
	case GraphicsMode::GL:
		textureSize = dynamic_cast<GLRenderManager*>(renderManager)->GetTexture(name_)->GetSize();
		break;
	case GraphicsMode::VK:
		textureSize = dynamic_cast<VKRenderManager*>(renderManager)->GetTexture(name_)->GetSize();
		break;
	}
	AddSpriteToManager();
}

void Sprite::AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color)
{
	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
		[=](Sprite* sprite) { sprite->CreateMesh3D(type, path, stacks_, slices_, color); });
}

void Sprite::AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
{
	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
		[=](Sprite* sprite) { sprite->CreateMesh3D(type, path, stacks_, slices_, color, metallic_, roughness_); });
}

void Sprite::RecreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color)
{
	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
		[](Sprite* sprite) { sprite->DeleteFromSpriteManagerList(); });
	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
		[=](Sprite* sprite) { sprite->CreateMesh3D(type, path, stacks_, slices_, color); });
	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
		[](Sprite* sprite) { sprite->SetIsTex(sprite->GetIsTex()); });
}

void Sprite::RecreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
{
	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
		[](Sprite* sprite) { sprite->DeleteFromSpriteManagerList(); });
	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
		[=](Sprite* sprite) { sprite->CreateMesh3D(type, path, stacks_, slices_, color, metallic_, roughness_); });
	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
		[](Sprite* sprite) { sprite->SetIsTex(sprite->GetIsTex()); });
}


void Sprite::CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color)
{
	meshType = type;
	filePath = path;
	stacks = stacks_;
	slices = slices_;

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->LoadMesh(type, path, color, stacks, slices);
	//materialId = Engine::GetSpriteManager().GetSpritesAmount();
	materialId = static_cast<int>(Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->size() - 1);
	SetSpriteDrawType(SpriteDrawType::ThreeDimension);
	AddSpriteToManager();
}

void Sprite::CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
{
	meshType = type;
	filePath = path;
	stacks = stacks_;
	slices = slices_;

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->LoadMesh(type, path, color, stacks_, slices_, metallic_, roughness_);
	//materialId = Engine::GetSpriteManager().GetSpritesAmount();
	materialId = static_cast<int>(Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->size() - 1);
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
	AddMeshWithTexel(name);

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
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).frameSize = glm::vec3(GetFrameSize() / textureSize, 0.f);
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).texelPos = glm::vec3(GetFrameTexel(animations[currAnim]->GetDisplayFrame()) / textureSize, 0.f);
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
		if(spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			if (renderManagerGL->GetTexture(name) != nullptr)
			{
				renderManagerGL->GetFragmentUniforms2D()->at(materialId).texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				renderManagerGL->GetVertexUniforms2D()->at(materialId).isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				renderManagerGL->GetVertexUniforms2D()->at(materialId).isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerGL->GetTexture(name) != nullptr)
			{
				renderManagerGL->GetFragmentUniforms3D()->at(materialId).texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				renderManagerGL->GetFragmentUniforms3D()->at(materialId).isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				renderManagerGL->GetFragmentUniforms3D()->at(materialId).isTex = false;
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
				renderManagerVK->GetFragmentUniforms2D()->at(materialId).texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				renderManagerVK->GetVertexUniforms2D()->at(materialId).isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				renderManagerVK->GetVertexUniforms2D()->at(materialId).isTex = false;
				isTex = false;
			}
		}
		else 
		{
			if (renderManagerVK->GetTexture(name) != nullptr)
			{
				renderManagerVK->GetFragmentUniforms3D()->at(materialId).texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				renderManagerVK->GetFragmentUniforms3D()->at(materialId).isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				renderManagerVK->GetFragmentUniforms3D()->at(materialId).isTex = false;
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
	if(spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).color = color;
	}
	else
	{
		Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at(materialId).color = color;
	}
}

glm::vec4 Sprite::GetColor()
{
	if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		return Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at(materialId).color;
	}
	else
	{
		return Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at(materialId).color;
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
		GLRenderManager* renderManagerGL = dynamic_cast<GLRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			renderManagerGL->GetVertexUniforms2D()->at(materialId).isTex = state;
		}
		else
		{
			renderManagerGL->GetFragmentUniforms3D()->at(materialId).isTex = state;
		}
		break;
	}
	case GraphicsMode::VK:
	{
		VKRenderManager* renderManagerVK = dynamic_cast<VKRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			renderManagerVK->GetVertexUniforms2D()->at(materialId).isTex = state;
		}
		else
		{
			renderManagerVK->GetFragmentUniforms3D()->at(materialId).isTex = state;
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
