//Author: JEYOON YU
//Project: CubeEngine
//File: DX2DRenderContext.cpp
#include "DX2DRenderContext.hpp"
#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
#include "Engine.hpp"

void DX2DRenderContext::Initialize()
{
	// Create root signature and pipeline for 2D
	// The slot of a root signature version 1.1
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(3, CD3DX12_ROOT_PARAMETER1{});
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_DESCRIPTOR_RANGE1 texSrvRange;
	texSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_OBJECT_SIZE, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	rootParameters[2].InitAsDescriptorTable(1, &texSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	m_renderManager->CreateRootSignature(m_rootSignature2D, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature2D->SetName(L"2D Root Signature"));

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32_UINT, 0, offsetof(TwoDimension::Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };
	m_pipeline2D = DXPipeLineBuilder(m_renderManager->m_device, m_rootSignature2D)
		.SetShaders("../Engine/shaders/hlsl/2D.vert.hlsl", "../Engine/shaders/hlsl/2D.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, true)
		.SetDepthStencil(true, true)
		.SetRenderTargets(rtvFormats)
		.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.Build();
}

void DX2DRenderContext::OnResize()
{

}

void DX2DRenderContext::Execute(ICommandListWrapper* commandListWrapper)
{
	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderManager->m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(m_renderManager->m_frameIndex), m_renderManager->m_rtvDescriptorSize);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_renderManager->m_renderTarget->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();
	m_renderManager->m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	commandList->SetPipelineState(m_pipeline2D->GetPipelineState().Get());
	commandList->SetGraphicsRootSignature(m_rootSignature2D.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* ppHeaps2D[] = { m_renderManager->m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps2D), ppHeaps2D);
	commandList->SetGraphicsRootDescriptorTable(2, m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart());

	std::vector<DynamicSprite*> sprites = Engine::Instance().GetSpriteManager().GetDynamicSprites();
	for (const auto& sprite : sprites)
	{
		for (auto& subMesh : sprite->GetSubMeshes())
		{
			auto* spriteData = subMesh->GetData<BufferWrapper::DynamicSprite2D>();
			auto* buffer = subMesh->GetBuffer<BufferWrapper::DXBuffer>();

			// Update Constant Buffer
			spriteData->GetVertexUniformBuffer<DXConstantBuffer<TwoDimension::VertexUniform>>()->UpdateConstant(&spriteData->vertexUniform, sizeof(TwoDimension::VertexUniform), m_renderManager->m_frameIndex);
			spriteData->GetFragmentUniformBuffer<DXConstantBuffer<TwoDimension::FragmentUniform>>()->UpdateConstant(&spriteData->fragmentUniform, sizeof(TwoDimension::FragmentUniform), m_renderManager->m_frameIndex);

			// Bind Vertex Buffer & Index Buffer
			D3D12_VERTEX_BUFFER_VIEW vbv = buffer->vertexBuffer->GetView();
			D3D12_INDEX_BUFFER_VIEW ibv = buffer->indexBuffer->GetView();
			commandList->IASetVertexBuffers(0, 1, &vbv);
			commandList->IASetIndexBuffer(&ibv);
			// Bind constant buffers to root signature
			commandList->SetGraphicsRootConstantBufferView(0, spriteData->GetVertexUniformBuffer<DXConstantBuffer<TwoDimension::VertexUniform>>()->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
			commandList->SetGraphicsRootConstantBufferView(1, spriteData->GetFragmentUniformBuffer<DXConstantBuffer<TwoDimension::FragmentUniform>>()->GetGPUVirtualAddress(m_renderManager->m_frameIndex));

			commandList->DrawIndexedInstanced(static_cast<UINT>(spriteData->indices.size()), 1, 0, 0, 0);
		}
	}
}

void DX2DRenderContext::CleanUp()
{
}
