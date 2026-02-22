//Author: JEYOON YU
//Project: CubeEngine
//File: DXForwardRenderContext.cpp
#include "DXForwardRenderContext.hpp"
#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
#include "DXSkyboxRenderContext.hpp"
#include "Engine.hpp"

void DXForwardRenderContext::Initialize()
{
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
	CD3DX12_DESCRIPTOR_RANGE1 iblSrvRange;
	iblSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	rootParameters[7].InitAsDescriptorTable(1, &iblSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);

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
	DXHelper::ThrowIfFailed(m_rootSignature3D->SetName(L"3D Root Signature"));

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32_UINT, 0, offsetof(ThreeDimension::QuantizedVertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout normalLayout{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ThreeDimension::QuantizedVertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout uvLayout{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ThreeDimension::QuantizedVertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout texSubIndexLayout{ "TEXCOORD", 1, DXGI_FORMAT_R32_SINT, 0, offsetof(ThreeDimension::QuantizedVertex, texSubIndex), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout boneIndexLayout{ "BLENDINDICES", 0, DXGI_FORMAT_R32_SINT, 0, offsetof(ThreeDimension::QuantizedVertex, boneIDs), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout weightLayout{ "BLENDWEIGHTS", 0, DXGI_FORMAT_R32_FLOAT, 0, offsetof(ThreeDimension::QuantizedVertex, weights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = m_renderManager->m_renderTarget->GetMSAASampleCount();
	sampleDesc.Quality = m_renderManager->m_renderTarget->GetMSAAQualityLevel();

	std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	if (m_renderManager->m_meshShaderEnabled)
	{
		m_meshPipeline3D = std::make_unique<DXMeshPipeLine>(
			m_renderManager->m_device,
			m_rootSignature3D,
			std::filesystem::path("../Engine/shaders/cso/3D.mesh.cso"),
			std::filesystem::path("../Engine/shaders/cso/3DMesh.frag.cso"),
			D3D12_FILL_MODE_SOLID,
			D3D12_CULL_MODE_BACK,
			sampleDesc,
			true,
			true,
			true,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
		);
	}
	else
	{
		m_pipeline3D = DXPipeLineBuilder(m_renderManager->m_device, m_rootSignature3D)
			.SetShaders("../Engine/shaders/hlsl/3D.vert.hlsl", "../Engine/shaders/hlsl/3D.frag.hlsl")
			.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout, normalLayout, uvLayout, texSubIndexLayout, boneIndexLayout, weightLayout })
			.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, true)
			.SetDepthStencil(true, true)
			.SetRenderTargets(rtvFormats)
			.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
			.SetBlendMode(CD3DX12_BLEND_DESC(D3D12_DEFAULT).RenderTarget[0])
			.SetSampleDesc(m_renderManager->m_renderTarget->GetMSAASampleCount(), m_renderManager->m_renderTarget->GetMSAAQualityLevel())
			.Build();

		// @TODO Separate to Wireframe Render Context (Debug Context)
		m_pipeline3DLine = DXPipeLineBuilder(m_renderManager->m_device, m_rootSignature3D)
			.SetShaders("../Engine/shaders/hlsl/3D.vert.hlsl", "../Engine/shaders/hlsl/3D.frag.hlsl")
			.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout, normalLayout, uvLayout, texSubIndexLayout, boneIndexLayout, weightLayout })
			.SetRasterizer(D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_BACK, true)
			.SetDepthStencil(true, true)
			.SetRenderTargets(rtvFormats)
			.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
			.SetBlendMode(CD3DX12_BLEND_DESC(D3D12_DEFAULT).RenderTarget[0])
			.SetSampleDesc(m_renderManager->m_renderTarget->GetMSAASampleCount(), m_renderManager->m_renderTarget->GetMSAAQualityLevel())
			.Build();
	}

	// @TODO Separate to Normal Vector Visualization Render Context (Debug Context)
#ifdef _DEBUG
	// Create root signature and pipeline for Normal 3D
	rootParameters.clear();
	rootParameters.resize(1, {});
	rootParameters[0].InitAsConstants(16, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	m_renderManager->CreateRootSignature(m_rootSignature3DNormal, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature3DNormal->SetName(L"Normal 3D Root Signature"));

	positionLayout.format = DXGI_FORMAT_R32G32B32_FLOAT;
	positionLayout.offset = offsetof(ThreeDimension::NormalVertex, position);

	m_pipeline3DNormal = DXPipeLineBuilder(m_renderManager->m_device, m_rootSignature3DNormal)
		.SetShaders("../Engine/shaders/hlsl/Normal3D.vert.hlsl", "../Engine/shaders/hlsl/Normal3D.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, true)
		.SetDepthStencil(true, true)
		.SetRenderTargets(rtvFormats)
		.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.SetBlendMode(CD3DX12_BLEND_DESC(D3D12_DEFAULT).RenderTarget[0])
		.SetSampleDesc(m_renderManager->m_renderTarget->GetMSAASampleCount(), m_renderManager->m_renderTarget->GetMSAAQualityLevel())
		.Build();
#endif
}

void DXForwardRenderContext::OnResize()
{

}

void DXForwardRenderContext::Execute(ICommandListWrapper* commandListWrapper)
{
	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_renderManager->m_renderTarget->GetMSAARtvHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_renderManager->m_renderTarget->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();
	// @TODO What does OMSetRenderTargets do?
	m_renderManager->m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	ID3D12PipelineState* initialState = m_renderManager->m_meshShaderEnabled ? m_meshPipeline3D->GetPipelineState().Get() :
		m_renderManager->pMode == PolygonType::FILL ? m_pipeline3D->GetPipelineState().Get() : m_pipeline3DLine->GetPipelineState().Get();
	commandList->SetPipelineState(initialState);

	commandList->SetGraphicsRootSignature(m_rootSignature3D.Get());

	// Set the viewport and scissor rect
	// @TODO This is weird but FidelityFX class takes care of viewport size (display size, render size)
	uint32_t renderWidth = m_renderManager->m_postProcessContext->GetFidelityFX()->GetRenderWidth();
	uint32_t renderHeight = m_renderManager->m_postProcessContext->GetFidelityFX()->GetRenderHeight();
	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(renderWidth), static_cast<FLOAT>(renderHeight), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(renderWidth), static_cast<LONG>(renderHeight) };

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	ID3D12DescriptorHeap* ppHeaps3D[] = { m_renderManager->m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps3D), ppHeaps3D);

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

				if (m_renderManager->directionalLightUniformBuffer && !m_renderManager->directionalLightUniforms.empty())
				{
					m_renderManager->directionalLightUniformBuffer->UpdateConstant(m_renderManager->directionalLightUniforms.data(), sizeof(ThreeDimension::DirectionalLightUniform) * m_renderManager->directionalLightUniforms.size(), m_renderManager->m_frameIndex);
					commandList->SetGraphicsRootConstantBufferView(3, m_renderManager->directionalLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
				}
				if (m_renderManager->pointLightUniformBuffer && !m_renderManager->pointLightUniforms.empty())
				{
					m_renderManager->pointLightUniformBuffer->UpdateConstant(m_renderManager->pointLightUniforms.data(), sizeof(ThreeDimension::PointLightUniform) * m_renderManager->pointLightUniforms.size(), m_renderManager->m_frameIndex);
					commandList->SetGraphicsRootConstantBufferView(4, m_renderManager->pointLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
				}

				m_renderManager->pushConstants = {
					.activeDirectionalLight = static_cast<int>(m_renderManager->directionalLightUniforms.size()),
					.activePointLight = static_cast<int>(m_renderManager->pointLightUniforms.size()),
				};
				commandList->SetGraphicsRoot32BitConstants(5, 2, &m_renderManager->pushConstants, 0);

				commandList->SetGraphicsRootDescriptorTable(6, m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart());

				if (m_renderManager->m_skyboxEnabled && m_renderManager->m_skyboxRenderContext->GetSkybox())
				{
					commandList->SetGraphicsRootDescriptorTable(7, m_renderManager->m_skyboxRenderContext->GetSkybox()->GetIrradianceMapSrv());
				}

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

				if (m_renderManager->directionalLightUniformBuffer && !m_renderManager->directionalLightUniforms.empty())
				{
					m_renderManager->directionalLightUniformBuffer->UpdateConstant(m_renderManager->directionalLightUniforms.data(), sizeof(ThreeDimension::DirectionalLightUniform) * m_renderManager->directionalLightUniforms.size(), m_renderManager->m_frameIndex);
					commandList->SetGraphicsRootConstantBufferView(3, m_renderManager->directionalLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
				}
				if (m_renderManager->pointLightUniformBuffer && !m_renderManager->pointLightUniforms.empty())
				{
					m_renderManager->pointLightUniformBuffer->UpdateConstant(m_renderManager->pointLightUniforms.data(), sizeof(ThreeDimension::PointLightUniform) * m_renderManager->pointLightUniforms.size(), m_renderManager->m_frameIndex);
					commandList->SetGraphicsRootConstantBufferView(4, m_renderManager->pointLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
				}

				m_renderManager->pushConstants = {
					.activeDirectionalLight = static_cast<int>(m_renderManager->directionalLightUniforms.size()),
					.activePointLight = static_cast<int>(m_renderManager->pointLightUniforms.size()),
				};
				commandList->SetGraphicsRoot32BitConstants(5, 2, &m_renderManager->pushConstants, 0);

				commandList->SetGraphicsRootDescriptorTable(6, m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart());

				if (m_renderManager->m_skyboxEnabled && m_renderManager->m_skyboxRenderContext->GetSkybox())
				{
					commandList->SetGraphicsRootDescriptorTable(7, m_renderManager->m_skyboxRenderContext->GetSkybox()->GetIrradianceMapSrv());
				}

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

#ifdef _DEBUG
			if (m_renderManager->m_normalVectorVisualization)
			{
				commandList->SetPipelineState(m_pipeline3DNormal->GetPipelineState().Get());
				commandList->SetGraphicsRootSignature(m_rootSignature3DNormal.Get());
				commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

				auto& vertexUniform = spriteData->vertexUniform;
				glm::mat4 modelToNDC = vertexUniform.projection * vertexUniform.view * vertexUniform.model;
				commandList->SetGraphicsRoot32BitConstants(0, 16, &modelToNDC, 0);

				D3D12_VERTEX_BUFFER_VIEW nvbv = buffer->normalVertexBuffer->GetView();
				commandList->IASetVertexBuffers(0, 1, &nvbv);

				commandList->DrawInstanced(static_cast<UINT>(spriteData->normalVertices.size()), 1, 0, 0);

				switch (m_renderManager->pMode)
				{
				case PolygonType::FILL:
					commandList->SetPipelineState(m_renderManager->m_meshShaderEnabled ? m_meshPipeline3D->GetPipelineState().Get() : m_pipeline3D->GetPipelineState().Get());
					break;
				case PolygonType::LINE:
					commandList->SetPipelineState(m_pipeline3DLine->GetPipelineState().Get());
					break;
				}
				commandList->SetGraphicsRootSignature(m_rootSignature3D.Get());
			}
#endif
		}
	}
}

void DXForwardRenderContext::CleanUp()
{
}
