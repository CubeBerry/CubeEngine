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
	if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		// Initialize VAO
		bufferWrapper.buffer = BufferWrapper::GLBuffer{};
		std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer).vertexArray.Initialize();
#ifdef _DEBUG
		std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer).normalVertexArray.Initialize();
#endif

//		auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
//		buffer.vertexBuffer = new GLVertexBuffer();
//#ifdef _DEBUG
//		buffer.normalVertexBuffer = new GLVertexBuffer();
//#endif
//		buffer.indexBuffer = new GLIndexBuffer(&indices);
//		buffer.vertexUniformBuffer = new GLUniformBuffer<VertexUniform>();
//		buffer.fragmentUniformBuffer = new GLUniformBuffer<FragmentUniform>();
//
//		RenderManager* renderManager = Engine::Instance().GetRenderManager();
//		dynamic_cast<GLRenderManager*>(renderManager)->InitializeBuffers(bufferWrapper, indices);
	}
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
		vertexUniform.vertex2D.model = modelMatrix;
		break;
	case SpriteDrawType::ThreeDimension:
		vertexUniform.vertex3D.model = modelMatrix;
		// @TODO move to push constants later
		vertexUniform.vertex3D.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
		break;
	case SpriteDrawType::UI:
		vertexUniform.vertex2D.model = modelMatrix;
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
		vertexUniform.vertex2D.model = modelMatrix;
		break;
	case SpriteDrawType::ThreeDimension:
		vertexUniform.vertex3D.model = modelMatrix;
		// @TODO move to push constants later
		vertexUniform.vertex3D.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
		break;
	case SpriteDrawType::UI:
		vertexUniform.vertex2D.model = modelMatrix;
		break;
	}
}

void Sprite::UpdateView()
{
	switch (spriteDrawType)
	{
	case SpriteDrawType::TwoDimension:
		vertexUniform.vertex2D.view = Engine::GetCameraManager().GetViewMatrix();
		break;
	case SpriteDrawType::ThreeDimension:
		vertexUniform.vertex3D.view = Engine::GetCameraManager().GetViewMatrix();
		// @TODO move to push constants later
		glm::mat4 inverseView = glm::inverse(vertexUniform.vertex3D.view);
		vertexUniform.vertex3D.viewPosition = glm::vec3(
			inverseView[3].x,
			inverseView[3].y,
			inverseView[3].z
			);
		break;
	case SpriteDrawType::UI:
		vertexUniform.vertex2D.view = glm::mat4(1.0f);
		break;
	}
}

void Sprite::UpdateProjection()
{
	switch (spriteDrawType)
	{
	case SpriteDrawType::TwoDimension:
		vertexUniform.vertex2D.projection = Engine::GetCameraManager().GetProjectionMatrix();
		break;
	case SpriteDrawType::ThreeDimension:
		vertexUniform.vertex3D.projection = Engine::GetCameraManager().GetProjectionMatrix();
		break;
	case SpriteDrawType::UI:
		glm::vec2 cameraViewSize = Engine::GetCameraManager().GetViewSize();
		vertexUniform.vertex2D.projection = glm::ortho(-cameraViewSize.x, cameraViewSize.x, -cameraViewSize.y, cameraViewSize.y, -1.f, 1.f);
		// Flip y-axis for Vulkan
		if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::VK)
		{
			vertexUniform.vertex2D.projection[1][1] *= -1;
		}
		break;
	}
}


void Sprite::AddQuad(glm::vec4 color_)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();

	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, -1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, -1.f, 1.f) });

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	vertexUniform.vertex2D.model = glm::mat4(1.f);
	vertexUniform.vertex2D.view = glm::mat4(1.f);
	vertexUniform.vertex2D.projection = glm::mat4(1.f);
	vertexUniform.vertex2D.color = color_;
	vertexUniform.vertex2D.isTex = 0.f;
	vertexUniform.vertex2D.isTexel = 0.f;

	fragmentUniform.frag2D.texIndex = 0;

	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		//bufferWrapper.buffer = BufferWrapper::GLBuffer{};
		//auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		//buffer.vertexBuffer = new GLVertexBuffer();
		//buffer.indexBuffer = new GLIndexBuffer(&indices);
		//buffer.vertexUniformBuffer = new GLUniformBuffer<VertexUniform>();
		//buffer.fragmentUniformBuffer = new GLUniformBuffer<FragmentUniform>();

		//vertexBuffer.buffer = VertexBufferWrapper::GLBuffer{};
		//std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer = new GLVertexBuffer();
		//indexBuffer.buffer = new GLIndexBuffer(&indices);
		//vertexUniformBuffer.buffer = new GLUniformBuffer<VertexUniform>();
		//fragmentUniformBuffer.buffer = new GLUniformBuffer<FragmentUniform>();

		dynamic_cast<GLRenderManager*>(renderManager)->InitializeBuffers(bufferWrapper, indices);
		auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::Float;
		position_layout.component_dimension = GLAttributeLayout::_3;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(TwoDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		buffer.vertexArray.AddVertexBuffer(std::move(*buffer.vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		buffer.vertexArray.SetIndexBuffer(std::move(*buffer.indexBuffer));

		//vertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		//vertexArray.SetIndexBuffer(std::move(*std::get<GLIndexBuffer*>(indexBuffer.buffer)));
	}
	else
	{
		bufferWrapper.buffer = BufferWrapper::VKBuffer{};
		auto& buffer = std::get<BufferWrapper::VKBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
		buffer.indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		buffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		buffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();

		//vertexBuffer.buffer = VertexBufferWrapper::VKBuffer{};
		//std::get<VertexBufferWrapper::VKBuffer>(vertexBuffer.buffer).vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
		//indexBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		//vertexUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		//fragmentUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
	}

	AddSpriteToManager();
}

void Sprite::AddQuadWithTexture(std::string name_, glm::vec4 color_)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();

	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, -1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, -1.f, 1.f) });

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	vertexUniform.vertex2D.model = glm::mat4(1.f);
	vertexUniform.vertex2D.view = glm::mat4(1.f);
	vertexUniform.vertex2D.projection = glm::mat4(1.f);
	vertexUniform.vertex2D.color = color_;
	vertexUniform.vertex2D.isTex = 0.f;
	vertexUniform.vertex2D.isTexel = 0.f;

	fragmentUniform.frag2D.texIndex = 0;

	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		//bufferWrapper.buffer = BufferWrapper::GLBuffer{};
		//auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		//buffer.vertexBuffer = new GLVertexBuffer();
		//buffer.indexBuffer = new GLIndexBuffer(&indices);
		//buffer.vertexUniformBuffer = new GLUniformBuffer<VertexUniform>();
		//buffer.fragmentUniformBuffer = new GLUniformBuffer<FragmentUniform>(); 

		//vertexBuffer.buffer = VertexBufferWrapper::GLBuffer{};
		//std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer = new GLVertexBuffer();
		//indexBuffer.buffer = new GLIndexBuffer(&indices);
		//vertexUniformBuffer.buffer = new GLUniformBuffer<VertexUniform>();
		//fragmentUniformBuffer.buffer = new GLUniformBuffer<FragmentUniform>();

		dynamic_cast<GLRenderManager*>(renderManager)->InitializeBuffers(bufferWrapper, indices);
		auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::Float;
		position_layout.component_dimension = GLAttributeLayout::_3;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(TwoDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		buffer.vertexArray.AddVertexBuffer(std::move(*buffer.vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		buffer.vertexArray.SetIndexBuffer(std::move(*buffer.indexBuffer));

		//vertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		//vertexArray.SetIndexBuffer(std::move(*std::get<GLIndexBuffer*>(indexBuffer.buffer)));
	}
	else
	{
		bufferWrapper.buffer = BufferWrapper::VKBuffer{};
		auto& buffer = std::get<BufferWrapper::VKBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
		buffer.indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		buffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		buffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();

		//vertexBuffer.buffer = VertexBufferWrapper::VKBuffer{};
		//std::get<VertexBufferWrapper::VKBuffer>(vertexBuffer.buffer).vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
		//indexBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		//vertexUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		//fragmentUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
	}

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

void Sprite::AddQuadWithTexel(std::string name_, glm::vec4 color_)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();

	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, -1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, -1.f, 1.f) });

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	vertexUniform.vertex2D.model = glm::mat4(1.f);
	vertexUniform.vertex2D.view = glm::mat4(1.f);
	vertexUniform.vertex2D.projection = glm::mat4(1.f);
	vertexUniform.vertex2D.color = color_;
	vertexUniform.vertex2D.isTex = 0.f;
	vertexUniform.vertex2D.isTexel = 0.f;

	fragmentUniform.frag2D.texIndex = 0;

	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		//bufferWrapper.buffer = BufferWrapper::GLBuffer{};
		//auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		//buffer.vertexBuffer = new GLVertexBuffer();
		//buffer.indexBuffer = new GLIndexBuffer(&indices);
		//buffer.vertexUniformBuffer = new GLUniformBuffer<VertexUniform>();
		//buffer.fragmentUniformBuffer = new GLUniformBuffer<FragmentUniform>();

		//vertexBuffer.buffer = VertexBufferWrapper::GLBuffer{};
		//std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer = new GLVertexBuffer();
		//indexBuffer.buffer = new GLIndexBuffer(&indices);
		//vertexUniformBuffer.buffer = new GLUniformBuffer<VertexUniform>();
		//fragmentUniformBuffer.buffer = new GLUniformBuffer<FragmentUniform>();

		dynamic_cast<GLRenderManager*>(renderManager)->InitializeBuffers(bufferWrapper, indices);
		auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::Float;
		position_layout.component_dimension = GLAttributeLayout::_3;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(TwoDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		buffer.vertexArray.AddVertexBuffer(std::move(*buffer.vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		buffer.vertexArray.SetIndexBuffer(std::move(*buffer.indexBuffer));

		//vertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		//vertexArray.SetIndexBuffer(std::move(*std::get<GLIndexBuffer*>(indexBuffer.buffer)));
	}
	else
	{
		bufferWrapper.buffer = BufferWrapper::VKBuffer{};
		auto& buffer = std::get<BufferWrapper::VKBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
		buffer.indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		buffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		buffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();

		//vertexBuffer.buffer = VertexBufferWrapper::VKBuffer{};
		//std::get<VertexBufferWrapper::VKBuffer>(vertexBuffer.buffer).vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
		//indexBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		//vertexUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		//fragmentUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
	}

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

//void Sprite::RecreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color)
//{
//	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
//		[](Sprite* sprite) { sprite->DeleteFromSpriteManagerList(); });
//	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
//		[=](Sprite* sprite) { sprite->CreateMesh3D(type, path, stacks_, slices_, color); });
//	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
//		[](Sprite* sprite) { sprite->SetIsTex(sprite->GetIsTex()); });
//}
//
//void Sprite::RecreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
//{
//	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
//		[](Sprite* sprite) { sprite->DeleteFromSpriteManagerList(); });
//	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
//		[=](Sprite* sprite) { sprite->CreateMesh3D(type, path, stacks_, slices_, color, metallic_, roughness_); });
//	Engine::GetObjectManager().QueueComponentFunction<Sprite>(this,
//		[](Sprite* sprite) { sprite->SetIsTex(sprite->GetIsTex()); });
//}


void Sprite::CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color)
{
	meshType = type;
	filePath = path;
	stacks = stacks_;
	slices = slices_;
	
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
#ifdef _DEBUG
	renderManager->CreateMesh(vertices, indices, normalVertices, type, path, stacks, slices);
#else
	renderManager->CreateMesh(vertices, indices, type, path, stacks, slices);
#endif

	vertexUniform.vertex3D.model = glm::mat4(1.f);
	vertexUniform.vertex3D.view = glm::mat4(1.f);
	vertexUniform.vertex3D.projection = glm::mat4(1.f);
	vertexUniform.vertex3D.color = color;

	fragmentUniform.frag3D.texIndex = 0;

	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		//bufferWrapper.buffer = BufferWrapper::GLBuffer{};
//		auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
//		buffer.vertexBuffer = new GLVertexBuffer();
//#ifdef _DEBUG
//		buffer.normalVertexBuffer = new GLVertexBuffer();
//#endif
//		buffer.indexBuffer = new GLIndexBuffer(&indices);
//		buffer.vertexUniformBuffer = new GLUniformBuffer<VertexUniform>();
//		buffer.fragmentUniformBuffer = new GLUniformBuffer<FragmentUniform>();

//		vertexBuffer.buffer = VertexBufferWrapper::GLBuffer{};
//		std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer = new GLVertexBuffer();
//#ifdef _DEBUG
//		std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).normalVertexBuffer = new GLVertexBuffer();
//#endif
//		indexBuffer.buffer = new GLIndexBuffer(&indices);
//		vertexUniformBuffer.buffer = new GLUniformBuffer<VertexUniform>();
//		fragmentUniformBuffer.buffer = new GLUniformBuffer<FragmentUniform>();

		dynamic_cast<GLRenderManager*>(renderManager)->InitializeBuffers(bufferWrapper, indices);
		auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * vertices.size()), vertices.data());
#ifdef _DEBUG
		buffer.normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * normalVertices.size()), normalVertices.data());
#endif

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::Float;
		position_layout.component_dimension = GLAttributeLayout::_3;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(ThreeDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(ThreeDimension::Vertex, position);

		GLAttributeLayout normal_layout;
		normal_layout.component_type = GLAttributeLayout::Float;
		normal_layout.component_dimension = GLAttributeLayout::_3;
		normal_layout.normalized = false;
		normal_layout.vertex_layout_location = 1;
		normal_layout.stride = sizeof(ThreeDimension::Vertex);
		normal_layout.offset = 0;
		normal_layout.relative_offset = offsetof(ThreeDimension::Vertex, normal);

		GLAttributeLayout uv_layout;
		uv_layout.component_type = GLAttributeLayout::Float;
		uv_layout.component_dimension = GLAttributeLayout::_2;
		uv_layout.normalized = false;
		uv_layout.vertex_layout_location = 2;
		uv_layout.stride = sizeof(ThreeDimension::Vertex);
		uv_layout.offset = 0;
		uv_layout.relative_offset = offsetof(ThreeDimension::Vertex, uv);

		GLAttributeLayout tex_sub_index_layout;
		tex_sub_index_layout.component_type = GLAttributeLayout::Int;
		tex_sub_index_layout.component_dimension = GLAttributeLayout::_1;
		tex_sub_index_layout.normalized = false;
		tex_sub_index_layout.vertex_layout_location = 3;
		tex_sub_index_layout.stride = sizeof(ThreeDimension::Vertex);
		tex_sub_index_layout.offset = 0;
		tex_sub_index_layout.relative_offset = offsetof(ThreeDimension::Vertex, texSubIndex);

		buffer.vertexArray.AddVertexBuffer(std::move(*buffer.vertexBuffer), sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
		buffer.vertexArray.SetIndexBuffer(std::move(*buffer.indexBuffer));

		//vertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer), sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
		//vertexArray.SetIndexBuffer(std::move(*std::get<GLIndexBuffer*>(indexBuffer.buffer)));

#ifdef _DEBUG
		GLAttributeLayout normal_position_layout;
		normal_position_layout.component_type = GLAttributeLayout::Float;
		normal_position_layout.component_dimension = GLAttributeLayout::_3;
		normal_position_layout.normalized = false;
		normal_position_layout.vertex_layout_location = 0;
		normal_position_layout.stride = sizeof(ThreeDimension::NormalVertex);
		normal_position_layout.offset = 0;
		normal_position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);

		GLAttributeLayout normal_color_layout;
		normal_color_layout.component_type = GLAttributeLayout::Float;
		normal_color_layout.component_dimension = GLAttributeLayout::_4;
		normal_color_layout.normalized = false;
		normal_color_layout.vertex_layout_location = 1;
		normal_color_layout.stride = sizeof(ThreeDimension::NormalVertex);
		normal_color_layout.offset = 0;
		normal_color_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, color);

		buffer.normalVertexArray.AddVertexBuffer(std::move(*buffer.normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout });
		//normalVertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout });
#endif
	}
	else
	{
		bufferWrapper.buffer = BufferWrapper::VKBuffer{};
		auto& buffer = std::get<BufferWrapper::VKBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
#ifdef _DEBUG
		buffer.normalVertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateNormalVertexBuffer(normalVertices);
#endif
		buffer.indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		buffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		buffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
		buffer.materialUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateMaterialUniformBuffer();

//		vertexBuffer.buffer = VertexBufferWrapper::VKBuffer{};
//		std::get<VertexBufferWrapper::VKBuffer>(vertexBuffer.buffer).vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
//#ifdef _DEBUG
//		std::get<VertexBufferWrapper::VKBuffer>(vertexBuffer.buffer).normalVertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateNormalVertexBuffer(normalVertices);
//#endif
//		indexBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
//		vertexUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
//		fragmentUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
//		materialUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateMaterialUniformBuffer();
	}

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
#ifdef _DEBUG
	renderManager->CreateMesh(vertices, indices, normalVertices, type, path, stacks, slices);
#else
	renderManager->CreateMesh(vertices, indices, type, path, stacks, slices);
#endif

	vertexUniform.vertex3D.model = glm::mat4(1.f);
	vertexUniform.vertex3D.view = glm::mat4(1.f);
	vertexUniform.vertex3D.projection = glm::mat4(1.f);
	vertexUniform.vertex3D.color = color;

	fragmentUniform.frag3D.texIndex = 0;

	material.metallic = metallic_;
	material.roughness = roughness_;

	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		//bufferWrapper.buffer = BufferWrapper::GLBuffer{};
//		auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
//		buffer.vertexBuffer = new GLVertexBuffer();
//#ifdef _DEBUG
//		buffer.normalVertexBuffer = new GLVertexBuffer();
//#endif
//		buffer.indexBuffer = new GLIndexBuffer(&indices);
//		buffer.vertexUniformBuffer = new GLUniformBuffer<VertexUniform>();
//		buffer.fragmentUniformBuffer = new GLUniformBuffer<FragmentUniform>();

//		vertexBuffer.buffer = VertexBufferWrapper::GLBuffer{};
//		std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer = new GLVertexBuffer();
//#ifdef _DEBUG
//		std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).normalVertexBuffer = new GLVertexBuffer();
//#endif
//		indexBuffer.buffer = new GLIndexBuffer(&indices);
//		vertexUniformBuffer.buffer = new GLUniformBuffer<VertexUniform>();
//		fragmentUniformBuffer.buffer = new GLUniformBuffer<FragmentUniform>();

		dynamic_cast<GLRenderManager*>(renderManager)->InitializeBuffers(bufferWrapper, indices);
		auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * vertices.size()), vertices.data());
#ifdef _DEBUG
		buffer.normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * normalVertices.size()), normalVertices.data());
#endif

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::Float;
		position_layout.component_dimension = GLAttributeLayout::_3;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(ThreeDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(ThreeDimension::Vertex, position);

		GLAttributeLayout normal_layout;
		normal_layout.component_type = GLAttributeLayout::Float;
		normal_layout.component_dimension = GLAttributeLayout::_3;
		normal_layout.normalized = false;
		normal_layout.vertex_layout_location = 1;
		normal_layout.stride = sizeof(ThreeDimension::Vertex);
		normal_layout.offset = 0;
		normal_layout.relative_offset = offsetof(ThreeDimension::Vertex, normal);

		GLAttributeLayout uv_layout;
		uv_layout.component_type = GLAttributeLayout::Float;
		uv_layout.component_dimension = GLAttributeLayout::_2;
		uv_layout.normalized = false;
		uv_layout.vertex_layout_location = 2;
		uv_layout.stride = sizeof(ThreeDimension::Vertex);
		uv_layout.offset = 0;
		uv_layout.relative_offset = offsetof(ThreeDimension::Vertex, uv);

		GLAttributeLayout tex_sub_index_layout;
		tex_sub_index_layout.component_type = GLAttributeLayout::Int;
		tex_sub_index_layout.component_dimension = GLAttributeLayout::_1;
		tex_sub_index_layout.normalized = false;
		tex_sub_index_layout.vertex_layout_location = 3;
		tex_sub_index_layout.stride = sizeof(ThreeDimension::Vertex);
		tex_sub_index_layout.offset = 0;
		tex_sub_index_layout.relative_offset = offsetof(ThreeDimension::Vertex, texSubIndex);

		buffer.vertexArray.AddVertexBuffer(std::move(*buffer.vertexBuffer), sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
		buffer.vertexArray.SetIndexBuffer(std::move(*buffer.indexBuffer));

		//vertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer), sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
		//vertexArray.SetIndexBuffer(std::move(*std::get<GLIndexBuffer*>(indexBuffer.buffer)));

#ifdef _DEBUG
		GLAttributeLayout normal_position_layout;
		normal_position_layout.component_type = GLAttributeLayout::Float;
		normal_position_layout.component_dimension = GLAttributeLayout::_3;
		normal_position_layout.normalized = false;
		normal_position_layout.vertex_layout_location = 0;
		normal_position_layout.stride = sizeof(ThreeDimension::NormalVertex);
		normal_position_layout.offset = 0;
		normal_position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);

		GLAttributeLayout normal_color_layout;
		normal_color_layout.component_type = GLAttributeLayout::Float;
		normal_color_layout.component_dimension = GLAttributeLayout::_4;
		normal_color_layout.normalized = false;
		normal_color_layout.vertex_layout_location = 1;
		normal_color_layout.stride = sizeof(ThreeDimension::NormalVertex);
		normal_color_layout.offset = 0;
		normal_color_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, color);

		buffer.normalVertexArray.AddVertexBuffer(std::move(*buffer.normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout });
		//normalVertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout });
#endif
	}
	else
	{
		bufferWrapper.buffer = BufferWrapper::VKBuffer{};
		auto& buffer = std::get<BufferWrapper::VKBuffer>(bufferWrapper.buffer);
		buffer.vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
#ifdef _DEBUG
		buffer.normalVertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateNormalVertexBuffer(normalVertices);
#endif
		buffer.indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		buffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		buffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
		buffer.materialUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateMaterialUniformBuffer();

//		vertexBuffer.buffer = VertexBufferWrapper::VKBuffer{};
//		std::get<VertexBufferWrapper::VKBuffer>(vertexBuffer.buffer).vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
//#ifdef _DEBUG
//		std::get<VertexBufferWrapper::VKBuffer>(vertexBuffer.buffer).normalVertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateNormalVertexBuffer(normalVertices);
//#endif
//		indexBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
//		vertexUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
//		fragmentUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
//		materialUniformBuffer.buffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateMaterialUniformBuffer();
	}

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
	AddQuadWithTexel(name);

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
		vertexUniform.vertex2D.frameSize = glm::vec3(GetFrameSize() / textureSize, 0.f);
		vertexUniform.vertex2D.texelPos = glm::vec3(GetFrameTexel(animations[currAnim]->GetDisplayFrame()) / textureSize, 0.f);
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
				fragmentUniform.frag2D.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				vertexUniform.vertex2D.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				vertexUniform.vertex2D.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerGL->GetTexture(name) != nullptr)
			{
				fragmentUniform.frag3D.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				fragmentUniform.frag3D.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				fragmentUniform.frag3D.isTex = false;
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
				fragmentUniform.frag2D.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				vertexUniform.vertex2D.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				vertexUniform.vertex2D.isTex = false;
				isTex = false;
			}
		}
		else 
		{
			if (renderManagerVK->GetTexture(name) != nullptr)
			{
				fragmentUniform.frag3D.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				fragmentUniform.frag3D.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				fragmentUniform.frag3D.isTex = false;
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
		vertexUniform.vertex2D.color = color;
	}
	else
	{
		vertexUniform.vertex3D.color = color;
	}
}

glm::vec4 Sprite::GetColor()
{
	if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		return vertexUniform.vertex2D.color;
	}
	else
	{
		return vertexUniform.vertex3D.color;
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
			vertexUniform.vertex2D.isTex = state;
		}
		else
		{
			fragmentUniform.frag3D.isTex = state;
		}
		break;
	}
	case GraphicsMode::VK:
	{
		//VKRenderManager* renderManagerVK = dynamic_cast<VKRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			vertexUniform.vertex2D.isTex = state;
		}
		else
		{
			fragmentUniform.frag3D.isTex = state;
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
