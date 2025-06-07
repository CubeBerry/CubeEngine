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
	}

	switch (spriteDrawType)
	{
	case SpriteDrawType::TwoDimension:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
		break;
	case SpriteDrawType::ThreeDimension:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.model = modelMatrix;
		// @TODO move to push constants later
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
		break;
	case SpriteDrawType::UI:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
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
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
		break;
	case SpriteDrawType::ThreeDimension:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.model = modelMatrix;
		// @TODO move to push constants later
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.transposeInverseModel = glm::transpose(glm::inverse(modelMatrix));
		break;
	case SpriteDrawType::UI:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.model = modelMatrix;
		break;
	}
}

void Sprite::UpdateView()
{
	switch (spriteDrawType)
	{
	case SpriteDrawType::TwoDimension:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.view = Engine::GetCameraManager().GetViewMatrix();
		break;
	case SpriteDrawType::ThreeDimension:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.view = Engine::GetCameraManager().GetViewMatrix();
		// @TODO move to push constants later
		glm::mat4 inverseView = glm::inverse(bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.view);
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.viewPosition = glm::vec3(
			inverseView[3].x,
			inverseView[3].y,
			inverseView[3].z
		);
		break;
	case SpriteDrawType::UI:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.view = glm::mat4(1.0f);
		break;
	}
}

void Sprite::UpdateProjection()
{
	switch (spriteDrawType)
	{
	case SpriteDrawType::TwoDimension:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.projection = Engine::GetCameraManager().GetProjectionMatrix();
		break;
	case SpriteDrawType::ThreeDimension:
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform.projection = Engine::GetCameraManager().GetProjectionMatrix();
		break;
	case SpriteDrawType::UI:
		glm::vec2 cameraViewSize = Engine::GetCameraManager().GetViewSize();
		bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.projection = glm::ortho(-cameraViewSize.x, cameraViewSize.x, -cameraViewSize.y, cameraViewSize.y, -1.f, 1.f);
		// Flip y-axis for Vulkan
		if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::VK)
		{
			bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform.projection[1][1] *= -1;
		}
		break;
	}
}


void Sprite::AddQuad(glm::vec4 color_)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	bufferWrapper.Initialize(Engine::GetRenderManager()->GetGraphicsMode(), RenderType::TwoDimension);

	auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, -1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, -1.f, 1.f) });

	auto& indices = bufferWrapper.GetIndices();
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.color = color_;
	vertexUniform.isTex = 0.f;
	vertexUniform.isTexel = 0.f;

	auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;
	fragmentUniform.texIndex = 0;

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
		//auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		//// @TODO How to avoid iterate vertices and resolve std::variant?
		//std::vector<TwoDimension::Vertex> rawVertices;
		//for (const auto& vertex : vertices)
		//{
		//	rawVertices.push_back(std::get<TwoDimension::Vertex>(vertex));
		//}
		//auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::Float;
		position_layout.component_dimension = GLAttributeLayout::_3;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		// @TODO Why ThreeDimension stride?
		position_layout.stride = sizeof(ThreeDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.AddVertexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.SetIndexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));

		//vertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		//vertexArray.SetIndexBuffer(std::move(*std::get<GLIndexBuffer*>(indexBuffer.buffer)));
	}
	else
	{
		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>() = BufferWrapper::VKBuffer{};
		//bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
		//bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		bufferWrapper.GetUniformBuffer<BufferWrapper::VKUniformBuffer2D>() = BufferWrapper::VKUniformBuffer2D{};
		//uniformBuffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		//uniformBuffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
	}

	SetSpriteDrawType(SpriteDrawType::TwoDimension);
	AddSpriteToManager();
}

void Sprite::AddQuadWithTexture(std::string name_, glm::vec4 color_)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	bufferWrapper.Initialize(Engine::GetRenderManager()->GetGraphicsMode(), RenderType::TwoDimension);

	auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, -1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, -1.f, 1.f) });

	auto& indices = bufferWrapper.GetIndices();
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.color = color_;
	vertexUniform.isTex = 0.f;
	vertexUniform.isTexel = 0.f;

	auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;
	fragmentUniform.texIndex = 0;

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
		//auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		// @TODO How to avoid iterate vertices and resolve std::variant?
		//std::vector<TwoDimension::Vertex> rawVertices;
		//for (const auto& vertex : vertices)
		//{
		//	rawVertices.push_back(std::get<TwoDimension::Vertex>(vertex));
		//}
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::Float;
		position_layout.component_dimension = GLAttributeLayout::_3;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		// @TODO Why ThreeDimension stride?
		position_layout.stride = sizeof(ThreeDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.AddVertexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.SetIndexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));

		//vertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		//vertexArray.SetIndexBuffer(std::move(*std::get<GLIndexBuffer*>(indexBuffer.buffer)));
	}
	else
	{
		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>() = BufferWrapper::VKBuffer{};
		//bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
		//bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		bufferWrapper.GetUniformBuffer<BufferWrapper::VKUniformBuffer2D>() = BufferWrapper::VKUniformBuffer2D{};
		//uniformBuffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		//uniformBuffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
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

	SetSpriteDrawType(SpriteDrawType::TwoDimension);
	AddSpriteToManager();
}

void Sprite::AddQuadWithTexel(std::string name_, glm::vec4 color_)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	bufferWrapper.Initialize(Engine::GetRenderManager()->GetGraphicsMode(), RenderType::TwoDimension);

	auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, 1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(1.f, -1.f, 1.f) });
	vertices.emplace_back(TwoDimension::Vertex{ glm::vec3(-1.f, -1.f, 1.f) });

	auto& indices = bufferWrapper.GetIndices();
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.color = color_;
	vertexUniform.isTex = 0.f;
	vertexUniform.isTexel = 1.f;

	auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;
	fragmentUniform.texIndex = 0;

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
		//auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		// @TODO How to avoid iterate vertices and resolve std::variant?
		//std::vector<TwoDimension::Vertex> rawVertices;
		//for (const auto& vertex : vertices)
		//{
		//	rawVertices.push_back(std::get<TwoDimension::Vertex>(vertex));
		//}
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::Float;
		position_layout.component_dimension = GLAttributeLayout::_3;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		// @TODO Why ThreeDimension stride?
		position_layout.stride = sizeof(ThreeDimension::Vertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.AddVertexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.SetIndexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));

		//vertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).vertexBuffer), sizeof(TwoDimension::Vertex), { position_layout });
		//vertexArray.SetIndexBuffer(std::move(*std::get<GLIndexBuffer*>(indexBuffer.buffer)));
	}
	else
	{
		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>() = BufferWrapper::VKBuffer{};
		//bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
		//bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		bufferWrapper.GetUniformBuffer<BufferWrapper::VKUniformBuffer2D>() = BufferWrapper::VKUniformBuffer2D{};
		//uniformBuffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		//uniformBuffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
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

	SetSpriteDrawType(SpriteDrawType::TwoDimension);
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
	bufferWrapper.Initialize(Engine::GetRenderManager()->GetGraphicsMode(), RenderType::ThreeDimension);
	auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertices;
	auto& normalVertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices;
	auto& indices = bufferWrapper.GetIndices();

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

	auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.color = color;

	auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;
	fragmentUniform.texIndex = 0;

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
		//auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);

		// @TODO How to avoid iterate vertices and resolve std::variant?
		//std::vector<ThreeDimension::Vertex> rawVertices;
		//for (const auto& vertex : vertices)
		//{
		//	rawVertices.push_back(std::get<ThreeDimension::Vertex>(vertex));
		//}
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * vertices.size()), vertices.data());
#ifdef _DEBUG
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * normalVertices.size()), normalVertices.data());
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

		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.AddVertexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.SetIndexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));

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

		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().normalVertexArray.AddVertexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout });
		//normalVertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout });
#endif
	}
	else
	{
		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>() = BufferWrapper::VKBuffer{};
//		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
//#ifdef _DEBUG
//		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().normalVertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateNormalVertexBuffer(normalVertices);
//#endif
//		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		//buffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		//buffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
		//buffer.materialUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateMaterialUniformBuffer();
	}

	SetSpriteDrawType(SpriteDrawType::ThreeDimension);
	AddSpriteToManager();
}

void Sprite::CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_)
{
	bufferWrapper.Initialize(Engine::GetRenderManager()->GetGraphicsMode(), RenderType::ThreeDimension);
	auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertices;
	auto& normalVertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices;
	auto& indices = bufferWrapper.GetIndices();

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

	auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.color = color;

	auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;
	fragmentUniform.texIndex = 0;

	auto& material = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().material;
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
		//auto& buffer = std::get<BufferWrapper::GLBuffer>(bufferWrapper.buffer);
		// @TODO How to avoid iterate vertices and resolve std::variant?
		//std::vector<ThreeDimension::Vertex> rawVertices;
		//for (const auto& vertex : vertices)
		//{
		//	rawVertices.push_back(std::get<ThreeDimension::Vertex>(vertex));
		//}
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * vertices.size()), vertices.data());
#ifdef _DEBUG
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * normalVertices.size()), normalVertices.data());
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

		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.AddVertexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexArray.SetIndexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));

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

		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().normalVertexArray.AddVertexBuffer(std::move(*bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout });
		//normalVertexArray.AddVertexBuffer(std::move(*std::get<VertexBufferWrapper::GLBuffer>(vertexBuffer.buffer).normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout });
#endif
	}
	else
	{
		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>() = BufferWrapper::VKBuffer{};
//		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().vertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexBuffer(vertices);
//#ifdef _DEBUG
//		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().normalVertexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateNormalVertexBuffer(normalVertices);
//#endif
//		bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>().indexBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateIndexBuffer(indices);
		//buffer.vertexUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateVertexUniformBuffer();
		//buffer.fragmentUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateFragmentUniformBuffer();
		//buffer.materialUniformBuffer = dynamic_cast<VKRenderManager*>(renderManager)->AllocateMaterialUniformBuffer();
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
		auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
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
				auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
				auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerGL->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerGL->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

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
				auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
				auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				vertexUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

				vertexUniform.isTex = false;
				isTex = false;
			}
		}
		else
		{
			if (renderManagerVK->GetTexture(name) != nullptr)
			{
				auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				fragmentUniform.texIndex = renderManagerVK->GetTexture(name)->GetTextrueId();
				fragmentUniform.isTex = true;
				isTex = true;
				textureName = name;
			}
			else
			{
				auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

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
		auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

		vertexUniform.color = color;
	}
	else
	{
		auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;

		vertexUniform.color = color;
	}
}

glm::vec4 Sprite::GetColor()
{
	if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
	{
		auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

		return vertexUniform.color;
	}
	else
	{
		auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;

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
			auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

			fragmentUniform.isTex = state;
		}
		break;
	}
	case GraphicsMode::VK:
	{
		//VKRenderManager* renderManagerVK = dynamic_cast<VKRenderManager*>(renderManager);
		if (spriteDrawType == SpriteDrawType::TwoDimension || spriteDrawType == SpriteDrawType::UI)
		{
			auto& vertexUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

			vertexUniform.isTex = state;
		}
		else
		{
			auto& fragmentUniform = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

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
