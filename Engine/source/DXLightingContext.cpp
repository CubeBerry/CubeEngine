//Author: JEYOON YU
//Project: CubeEngine
//File: DXLightingContext.cpp
#include "DXLightingContext.hpp"
#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
#include "DXSkyboxRenderContext.hpp"
#include "Engine.hpp"

void DXLightingContext::Initialize()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 7;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvHeap)));
	m_srvHeap->SetName(L"Lighting Context Integrated Heap");

	// Create root signature and pipeline
	// The slot of a root signature version 1.1
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
	rootParameters.resize(5);
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsConstants(sizeof(PushConstants) / 4, 2, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_DESCRIPTOR_RANGE1 gBufferSrvRange;
	gBufferSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 1);
	rootParameters[3].InitAsDescriptorTable(1, &gBufferSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_DESCRIPTOR_RANGE1 iblRange;
	iblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 2);
	rootParameters[4].InitAsDescriptorTable(1, &iblRange, D3D12_SHADER_VISIBILITY_PIXEL);

	m_renderManager->CreateRootSignature(m_rootSignature, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature->SetName(L"Lighting-Pass Root Signature"));

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	m_pipeline = std::make_unique<DXPipeLine>(
		m_renderManager->m_device,
		m_rootSignature,
		std::filesystem::path("../Engine/shaders/hlsl/LightingPass.vert.hlsl"),
		std::filesystem::path("../Engine/shaders/hlsl/LightingPass.frag.hlsl"),
		std::initializer_list<DXAttributeLayout>{},
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK,
		sampleDesc,
		false,
		false,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);
}

void DXLightingContext::OnResize()
{

}

void DXLightingContext::Execute(ICommandListWrapper* commandListWrapper)
{
	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	// Copy G-Buffer SRV and IBL SRV to local heap
	ID3D12Device14* device = m_renderManager->m_device.Get();
	UINT srvSize = m_renderManager->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dstHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE srcGBuffer(
		m_renderManager->GetGBufferContext()->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart()
	);
	device->CopyDescriptorsSimple(4, dstHandle, srcGBuffer, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	dstHandle.Offset(4, srvSize);
	if (m_renderManager->m_skyboxEnabled && m_renderManager->m_skyboxRenderContext->GetSkybox())
	{
		D3D12_CPU_DESCRIPTOR_HANDLE srcIBL = m_renderManager->m_skyboxRenderContext->GetSkybox()->GetIrradianceMapCpuHandle();
		device->CopyDescriptorsSimple(3, dstHandle, srcIBL, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	ID3D12PipelineState* initialState = m_pipeline->GetPipelineState().Get();
	commandList->SetPipelineState(initialState);

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	if (m_renderManager->directionalLightUniformBuffer && !m_renderManager->directionalLightUniforms.empty())
	{
		m_renderManager->directionalLightUniformBuffer->UpdateConstant(m_renderManager->directionalLightUniforms.data(), sizeof(ThreeDimension::DirectionalLightUniform) * m_renderManager->directionalLightUniforms.size(), m_renderManager->m_frameIndex);
		commandList->SetGraphicsRootConstantBufferView(0, m_renderManager->directionalLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
	}
	if (m_renderManager->pointLightUniformBuffer && !m_renderManager->pointLightUniforms.empty())
	{
		m_renderManager->pointLightUniformBuffer->UpdateConstant(m_renderManager->pointLightUniforms.data(), sizeof(ThreeDimension::PointLightUniform) * m_renderManager->pointLightUniforms.size(), m_renderManager->m_frameIndex);
		commandList->SetGraphicsRootConstantBufferView(1, m_renderManager->pointLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
	}

	pushConstants.activeDirectionalLight = static_cast<int>(m_renderManager->directionalLightUniforms.size());
	pushConstants.activePointLight = static_cast<int>(m_renderManager->pointLightUniforms.size());
	glm::mat4 inverseView = glm::inverse(Engine::GetCameraManager().GetViewMatrix());
	pushConstants.viewPosition = glm::vec3(
		inverseView[3].x,
		inverseView[3].y,
		inverseView[3].z
	);
	commandList->SetGraphicsRoot32BitConstants(2, sizeof(PushConstants) / 4, &pushConstants, 0);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->SetGraphicsRootDescriptorTable(3, gpuHandle);
	gpuHandle.Offset(4, srvSize);
	if (m_renderManager->m_skyboxEnabled) commandList->SetGraphicsRootDescriptorTable(4, gpuHandle);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);
}

void DXLightingContext::CleanUp()
{
}
