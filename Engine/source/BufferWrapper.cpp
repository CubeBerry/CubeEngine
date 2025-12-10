//Author: JEYOON YU
//Project: CubeEngine
//File: BufferWrapper.cpp
#include "BufferWrapper.hpp"
#include "Engine.hpp"

BufferWrapper::~BufferWrapper() = default;

//BufferWrapper::~BufferWrapper()
//{
	// Deallocate SRV block
	//RenderManager* renderManager = Engine::GetRenderManager();
	//if (renderManager->GetGraphicsMode() == GraphicsMode::DX)
	//{
	//	dynamic_cast<DXRenderManager*>(renderManager)->DeallocateSrvBlock(std::get<DXBuffer>(buffer).srvHandle.second, 4);

		//CD3DX12_CPU_DESCRIPTOR_HANDLE meshletSrvHandle(buffer.srvHandle.first, 0, m_srvDescriptorSize);
		//buffer.meshletBuffer = std::make_unique<DXStructuredBuffer<Meshlet::Meshlet>>(m_device, m_commandQueue, bufferData3D.Meshlets, meshletSrvHandle);

		//CD3DX12_CPU_DESCRIPTOR_HANDLE uniqueVertexIndexSrvHandle(buffer.srvHandle.first, 1, m_srvDescriptorSize);
		//buffer.uniqueVertexIndexBuffer = std::make_unique<DXStructuredBuffer<uint32_t>>(m_device, m_commandQueue, bufferData3D.UniqueVertexIndices, uniqueVertexIndexSrvHandle);

		//CD3DX12_CPU_DESCRIPTOR_HANDLE primitiveIndexSrvHandle(buffer.srvHandle.first, 2, m_srvDescriptorSize);
		//buffer.primitiveIndexBuffer = std::make_unique<DXStructuredBuffer<uint8_t>>(m_device, m_commandQueue, bufferData3D.PrimitiveIndices, primitiveIndexSrvHandle);
	//}

//	if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::DX)
//		dynamic_cast<DXRenderManager*>(Engine::GetRenderManager())->WaitForGPU();
//
//	std::visit([]<typename T>(T & buf)
//	{
//		if constexpr (!std::is_same_v<std::decay_t<T>, std::monostate>)
//		{
//			delete buf.vertexBuffer;
//#ifdef _DEBUG
//			delete buf.normalVertexBuffer;
//#endif
//			delete buf.indexBuffer;
//		}
//	}, buffer);
//
//	std::visit([]<typename T>(T & buf)
//	{
//		if constexpr (!std::is_same_v<std::decay_t<T>, std::monostate>)
//		{
//			delete buf.vertexUniformBuffer;
//			delete buf.fragmentUniformBuffer;
//			if constexpr (requires { buf.materialUniformBuffer; })
//			{
//				delete buf.materialUniformBuffer;
//			}
//		}
//	}, uniformBuffer);
//};

void BufferWrapper::DynamicSprite2D::Initialize()
{
	switch (Engine::GetRenderManager()->GetGraphicsMode())
	{
	case GraphicsMode::GL:
		vertexUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<TwoDimension::VertexUniform>>>();
		fragmentUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<TwoDimension::FragmentUniform>>>();
		break;
	case GraphicsMode::DX:
		vertexUniformBuffer.emplace<std::unique_ptr<DXConstantBuffer<TwoDimension::VertexUniform>>>();
		fragmentUniformBuffer.emplace<std::unique_ptr<DXConstantBuffer<TwoDimension::FragmentUniform>>>();
		break;
	default:
		break;
	}
}

void BufferWrapper::DynamicSprite3DMesh::Initialize()
{
	switch (Engine::GetRenderManager()->GetGraphicsMode())
	{
	case GraphicsMode::GL:
		vertexUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>>();
		fragmentUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<ThreeDimension::FragmentUniform>>>();
		materialUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<ThreeDimension::Material>>>();
		break;
	case GraphicsMode::DX:
		vertexUniformBuffer.emplace<std::unique_ptr<DXConstantBuffer<ThreeDimension::VertexUniform>>>();
		fragmentUniformBuffer.emplace<std::unique_ptr<DXConstantBuffer<ThreeDimension::FragmentUniform>>>();
		materialUniformBuffer.emplace<std::unique_ptr<DXConstantBuffer<ThreeDimension::Material>>>();
		break;
	default:
		break;
	}
}

void BufferWrapper::StaticSprite3D::Initialize()
{
	switch (Engine::GetRenderManager()->GetGraphicsMode())
	{
	case GraphicsMode::GL:
		vertexUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>>();
		fragmentUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<ThreeDimension::FragmentUniform>>>();
		materialUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<ThreeDimension::Material>>>();
		break;
	case GraphicsMode::DX:
		vertexUniformBuffer.emplace<std::unique_ptr<DXStructuredBuffer<ThreeDimension::VertexUniform>>>();
		fragmentUniformBuffer.emplace<std::unique_ptr<DXStructuredBuffer<ThreeDimension::FragmentUniform>>>();
		materialUniformBuffer.emplace<std::unique_ptr<DXStructuredBuffer<ThreeDimension::Material>>>();
		break;
	default:
		break;
	}
}

BufferWrapper::BufferWrapper(SpriteType spriteType, bool meshShaderEnabled)
{
	switch (Engine::GetRenderManager()->GetGraphicsMode())
	{
	case GraphicsMode::GL: buffer.emplace<GLBuffer>(); break;
	case GraphicsMode::VK: buffer.emplace<VKBuffer>(); break;
	case GraphicsMode::DX: buffer.emplace<DXBuffer>(); break;
	}

	if (Engine::GetRenderManager()->GetRenderType() == RenderType::TwoDimension)
	{
		auto& sprite = data.emplace<DynamicSprite2D>();
		sprite.Initialize();
	}
	else
	{
		if (spriteType == SpriteType::DYNAMIC)
		{
			// @TODO Temporarily used for both non-mesh shader and mesh shader
			//if (meshShaderEnabled)
			//{
			auto& sprite = data.emplace<DynamicSprite3DMesh>();
			sprite.Initialize();
			//}
			//else
			//{
			//	auto& sprite = data.emplace<DynamicSprite3D>();
			//	sprite.Initialize();
			//}
		}
		else if (spriteType == SpriteType::STATIC)
		{
			auto& sprite = data.emplace<StaticSprite3D>();
			sprite.Initialize();
		}
	}
}
