//Author: JEYOON YU
//Project: CubeEngine
//File: DXGBufferContext.cpp
#include "DXGBufferContext.hpp"
#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
#include "DXSkyboxRenderContext.hpp"
#include "Engine.hpp"

void DXGBufferContext::Initialize()
{
	// Create Descriptor Heaps
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = static_cast<UINT>(m_gBuffers.size()); // Albedo, Normal, World Position, Material
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	DXHelper::ThrowIfFailed(m_rtvHeap->SetName(L"G-Buffer Render Target View Heap"));

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = static_cast<UINT>(m_gBuffers.size()); // Albedo, Normal, World Position, Material
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
	DXHelper::ThrowIfFailed(m_srvHeap->SetName(L"G-Buffer Shader Resource View Heap"));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	DXHelper::ThrowIfFailed(m_dsvHeap->SetName(L"G-Buffer Depth Stencil View Heap"));

	UINT rtvDescriptorSize = m_renderManager->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT srvDescriptorSize = m_renderManager->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart());

	int width = m_renderManager->m_width;
	int height = m_renderManager->m_height;

	// Create Depth Stencil Buffer Resource
	{
		D3D12_RESOURCE_DESC depthStencilDesc = {};
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = static_cast<UINT64>(width);
		depthStencilDesc.Height = static_cast<UINT>(height);
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		// @TODO Use ID3D12Device::CheckFeatureSupport to find supported format
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&m_depthStencil)
		));
		DXHelper::ThrowIfFailed(m_depthStencil->SetName(L"G-Buffer Depth/Stencil View"));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		m_renderManager->m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Create G-Buffer Render Targets
	for (auto& gBuffer : m_gBuffers)
	{
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Alignment = 0;
		textureDesc.Width = static_cast<UINT64>(width);
		textureDesc.Height = static_cast<UINT>(height);
		textureDesc.DepthOrArraySize = 1;
		textureDesc.MipLevels = 1;
		textureDesc.Format = gBuffer.format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = gBuffer.format;
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 1.0f;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&clearValue,
			IID_PPV_ARGS(&gBuffer.resource)
		));
		DXHelper::ThrowIfFailed(gBuffer.resource->SetName(gBuffer.name.c_str()));

		m_renderManager->m_device->CreateRenderTargetView(gBuffer.resource.Get(), nullptr, rtvHandle);
		m_renderManager->m_device->CreateShaderResourceView(gBuffer.resource.Get(), nullptr, srvHandle);

		// Move to the next descriptor
		rtvHandle.Offset(1, rtvDescriptorSize);
		srvHandle.Offset(1, srvDescriptorSize);
	}

	// Create root signature and pipeline for 3D
	// The slot of a root signature version 1.1
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
	rootParameters.resize(m_renderManager->m_meshShaderEnabled ? 13 : 8, CD3DX12_ROOT_PARAMETER1{});
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[3].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[4].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[5].InitAsConstants(2, 5, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_DESCRIPTOR_RANGE1 texSrvRange;
	texSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_OBJECT_SIZE, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	rootParameters[6].InitAsDescriptorTable(1, &texSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	if (m_renderManager->m_meshShaderEnabled)
	{
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_MESH);
		// @TODO CD3DX12_DESCRIPTOR_RANGE1 srvTableRange; can be used here
		// ThreeDimension::QuantizedVertex data is transfer via SRV
		rootParameters[8].InitAsShaderResourceView(8, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_MESH);
		rootParameters[9].InitAsShaderResourceView(9, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_MESH);
		rootParameters[10].InitAsShaderResourceView(10, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_MESH);
		rootParameters[11].InitAsShaderResourceView(11, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_MESH);
		rootParameters[12].InitAsConstants(1, 6, 0, D3D12_SHADER_VISIBILITY_MESH);
	}

	m_renderManager->CreateRootSignature(m_rootSignature3D, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature3D->SetName(L"G-Buffer Root Signature"));

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32_UINT, 0, offsetof(ThreeDimension::QuantizedVertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout normalLayout{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ThreeDimension::QuantizedVertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout uvLayout{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ThreeDimension::QuantizedVertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout texSubIndexLayout{ "TEXCOORD", 1, DXGI_FORMAT_R32_SINT, 0, offsetof(ThreeDimension::QuantizedVertex, texSubIndex), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	// Turn off MSAA for G-Buffer pass
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	std::vector<DXGI_FORMAT> rtvFormats = {
		m_gBuffers[static_cast<size_t>(GBufferType::Albedo)].format,
		m_gBuffers[static_cast<size_t>(GBufferType::Normal)].format,
		m_gBuffers[static_cast<size_t>(GBufferType::WorldPosition)].format,
		m_gBuffers[static_cast<size_t>(GBufferType::Material)].format
	};

	if (m_renderManager->m_meshShaderEnabled)
	{
		m_meshPipeline3D = std::make_unique<DXMeshPipeLine>(
			m_renderManager->m_device,
			m_rootSignature3D,
			std::filesystem::path("../Engine/shaders/cso/3D.mesh.cso"),
			std::filesystem::path("../Engine/shaders/cso/GBufferMesh.frag.cso"),
			D3D12_FILL_MODE_SOLID,
			D3D12_CULL_MODE_BACK,
			sampleDesc,
			true,
			true,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
		);
	}
	else
	{
		m_pipeline3D = std::make_unique<DXPipeLine>(
			m_renderManager->m_device,
			m_rootSignature3D,
			std::filesystem::path("../Engine/shaders/hlsl/3D.vert.hlsl"),
			std::filesystem::path("../Engine/shaders/hlsl/GBuffer.frag.hlsl"),
			std::initializer_list<DXAttributeLayout>{ positionLayout, normalLayout, uvLayout, texSubIndexLayout },
			D3D12_FILL_MODE_SOLID,
			D3D12_CULL_MODE_BACK,
			sampleDesc,
			true,
			true,
			rtvFormats,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
		);
	}
}

void DXGBufferContext::OnResize()
{
}

void DXGBufferContext::Execute(ICommandListWrapper* commandListWrapper)
{
	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	ID3D12PipelineState* initialState = m_renderManager->m_meshShaderEnabled ? m_meshPipeline3D->GetPipelineState().Get() : m_pipeline3D->GetPipelineState().Get();
	commandList->SetPipelineState(initialState);

	commandList->SetGraphicsRootSignature(m_rootSignature3D.Get());
	ID3D12DescriptorHeap* ppHeaps3D[] = { m_renderManager->m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps3D), ppHeaps3D);

	std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
	for (const auto& gBuffer : m_gBuffers)
	{
		barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
			gBuffer.resource.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));
	}
	barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
		m_depthStencil.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	));
	commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());

	auto rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	UINT rtvDescriptorSize = m_renderManager->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Clear G-Buffer Render Targets
	for (size_t i = 0; i < m_gBuffers.size(); ++i)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE currentRtvHandle(rtvHandle, static_cast<INT>(i), rtvDescriptorSize);
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		commandList->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);
	}
	// Clear Depth Stencil
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
	for (size_t i = 0; i < m_gBuffers.size(); ++i)
	{
		rtvHandles.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHandle, static_cast<INT>(i), rtvDescriptorSize));
	}

	commandList->OMSetRenderTargets(static_cast<UINT>(rtvHandles.size()), rtvHandles.data(), FALSE, &dsvHandle);

	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<FLOAT>(m_renderManager->m_width), static_cast<FLOAT>(m_renderManager->m_height), 0.0f, 1.0f };
	D3D12_RECT scissorRect = { 0, 0, m_renderManager->m_width, m_renderManager->m_height };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	std::vector<DynamicSprite*> sprites = Engine::Instance().GetSpriteManager().GetDynamicSprites();
	for (const auto& sprite : sprites)
	{
		for (auto& subMesh : sprite->GetSubMeshes())
		{
			auto* spriteData = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>();
			auto* buffer = subMesh->GetBuffer<BufferWrapper::DXBuffer>();
			if (m_renderManager->m_meshShaderEnabled)
			{
				commandList->SetPipelineState(m_meshPipeline3D->GetPipelineState().Get());

				// Update Constant Buffer
				spriteData->GetVertexUniformBuffer<DXConstantBuffer<ThreeDimension::VertexUniform>>()->UpdateConstant(&spriteData->vertexUniform, sizeof(ThreeDimension::VertexUniform), m_renderManager->m_frameIndex);
				spriteData->GetFragmentUniformBuffer<DXConstantBuffer<ThreeDimension::FragmentUniform>>()->UpdateConstant(&spriteData->fragmentUniform, sizeof(ThreeDimension::FragmentUniform), m_renderManager->m_frameIndex);
				spriteData->GetMaterialUniformBuffer<DXConstantBuffer<ThreeDimension::Material>>()->UpdateConstant(&spriteData->material, sizeof(ThreeDimension::Material), m_renderManager->m_frameIndex);

				commandList->SetGraphicsRootConstantBufferView(0, spriteData->GetVertexUniformBuffer<DXConstantBuffer<ThreeDimension::VertexUniform>>()->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
				commandList->SetGraphicsRootConstantBufferView(1, spriteData->GetFragmentUniformBuffer<DXConstantBuffer<ThreeDimension::FragmentUniform>>()->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
				commandList->SetGraphicsRootConstantBufferView(2, spriteData->GetMaterialUniformBuffer<DXConstantBuffer<ThreeDimension::Material>>()->GetGPUVirtualAddress(m_renderManager->m_frameIndex));

				commandList->SetGraphicsRootDescriptorTable(6, m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart());

				// Bind structured buffers to root signature
				commandList->SetGraphicsRootShaderResourceView(8, buffer->uniqueVertexBuffer->GetGPUVirtualAddress());
				commandList->SetGraphicsRootShaderResourceView(9, buffer->meshletBuffer->GetGPUVirtualAddress());
				commandList->SetGraphicsRootShaderResourceView(10, buffer->uniqueVertexIndexBuffer->GetGPUVirtualAddress());
				commandList->SetGraphicsRootShaderResourceView(11, buffer->primitiveIndexBuffer->GetGPUVirtualAddress());
				commandList->SetGraphicsRoot32BitConstants(12, 1, &m_renderManager->m_meshletVisualization, 0);

				const auto& meshlets = spriteData->meshlets;
				UINT numMeshlets = static_cast<UINT>(meshlets.size());
				commandList->DispatchMesh(numMeshlets, 1, 1);
			}
			else
			{
				commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				// Update Constant Buffer
				spriteData->GetVertexUniformBuffer<DXConstantBuffer<ThreeDimension::VertexUniform>>()->UpdateConstant(&spriteData->vertexUniform, sizeof(ThreeDimension::VertexUniform), m_renderManager->m_frameIndex);
				spriteData->GetFragmentUniformBuffer<DXConstantBuffer<ThreeDimension::FragmentUniform>>()->UpdateConstant(&spriteData->fragmentUniform, sizeof(ThreeDimension::FragmentUniform), m_renderManager->m_frameIndex);
				spriteData->GetMaterialUniformBuffer<DXConstantBuffer<ThreeDimension::Material>>()->UpdateConstant(&spriteData->material, sizeof(ThreeDimension::Material), m_renderManager->m_frameIndex);

				commandList->SetGraphicsRoot32BitConstants(5, 2, &m_renderManager->pushConstants, 0);

				commandList->SetGraphicsRootDescriptorTable(6, m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart());

				// Bind Vertex Buffer & Index Buffer
				D3D12_VERTEX_BUFFER_VIEW vbv = buffer->vertexBuffer->GetView();
				D3D12_INDEX_BUFFER_VIEW ibv = buffer->indexBuffer->GetView();
				commandList->IASetVertexBuffers(0, 1, &vbv);
				commandList->IASetIndexBuffer(&ibv);
				// Bind constant buffers to root signature
				commandList->SetGraphicsRootConstantBufferView(0, spriteData->GetVertexUniformBuffer<DXConstantBuffer<ThreeDimension::VertexUniform>>()->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
				commandList->SetGraphicsRootConstantBufferView(1, spriteData->GetFragmentUniformBuffer<DXConstantBuffer<ThreeDimension::FragmentUniform>>()->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
				commandList->SetGraphicsRootConstantBufferView(2, spriteData->GetMaterialUniformBuffer<DXConstantBuffer<ThreeDimension::Material>>()->GetGPUVirtualAddress(m_renderManager->m_frameIndex));

				commandList->DrawIndexedInstanced(static_cast<UINT>(spriteData->indices.size()), 1, 0, 0, 0);
			}
		}
	}

	barriers.clear();
	// Transition G-Buffer Render Targets to Pixel Shader Resource for lighting pass
	// @TODO gBuffer needs to be D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE after lighting pass is implemented
	for (const auto& gBuffer : m_gBuffers)
	{
		barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
			gBuffer.resource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		));
	}
	barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
		m_depthStencil.Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_COMMON
	));
	commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
}

void DXGBufferContext::CleanUp()
{
}
