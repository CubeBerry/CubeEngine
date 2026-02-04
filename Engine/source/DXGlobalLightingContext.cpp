//Author: JEYOON YU
//Project: CubeEngine
//File: DXGlobalLightingContext.cpp
#include "DXGlobalLightingContext.hpp"
#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
#include "DXSkyboxRenderContext.hpp"
#include "Engine.hpp"

void DXGlobalLightingContext::Initialize()
{
	// Copy G-Buffer SRV handles to Main SRV Heap
	m_gBufferSrvHandle = m_renderManager->AllocateSrvHandles(4);
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_gBufferSrvHandle.first;
	D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = m_renderManager->GetGBufferContext()->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	m_renderManager->m_device->CopyDescriptorsSimple(4, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Create root signature and pipeline
	// The slot of a root signature version 1.1
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
	rootParameters.resize(4);
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[1].InitAsConstants(sizeof(PushConstants) / 4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_DESCRIPTOR_RANGE1 gBufferSrvRange;
	gBufferSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 1);
	rootParameters[2].InitAsDescriptorTable(1, &gBufferSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_DESCRIPTOR_RANGE1 iblRange;
	iblRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 2);
	rootParameters[3].InitAsDescriptorTable(1, &iblRange, D3D12_SHADER_VISIBILITY_PIXEL);

	m_renderManager->CreateRootSignature(m_rootSignature, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature->SetName(L"Global Lighting-Pass Root Signature"));

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	m_pipeline = std::make_unique<DXPipeLine>(
		m_renderManager->m_device,
		m_rootSignature,
		std::filesystem::path("../Engine/shaders/hlsl/GlobalLightingPass.vert.hlsl"),
		std::filesystem::path("../Engine/shaders/hlsl/GlobalLightingPass.frag.hlsl"),
		std::initializer_list<DXAttributeLayout>{},
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK,
		sampleDesc,
		CD3DX12_BLEND_DESC(D3D12_DEFAULT).RenderTarget[0],
		false,
		false,
		false,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);
}

void DXGlobalLightingContext::OnResize()
{
	// Copy G-Buffer SRV handles to Main SRV Heap
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_gBufferSrvHandle.first;
	D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = m_renderManager->GetGBufferContext()->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	m_renderManager->m_device->CopyDescriptorsSimple(4, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DXGlobalLightingContext::Execute(ICommandListWrapper* commandListWrapper)
{
	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	ID3D12PipelineState* initialState = m_pipeline->GetPipelineState().Get();
	commandList->SetPipelineState(initialState);

	// It is already in RENDER_TARGET state from DXRenderManager::BeginRender
	//auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
	//	m_renderManager->m_renderTarget->GetRenderTarget().Get(),
	//	D3D12_RESOURCE_STATE_COMMON,
	//	D3D12_RESOURCE_STATE_RENDER_TARGET
	//);
	//commandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_renderManager->m_renderTarget->GetHDRRtvHeap()->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { m_renderManager->m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	if (m_renderManager->directionalLightUniformBuffer && !m_renderManager->directionalLightUniforms.empty())
	{
		m_renderManager->directionalLightUniformBuffer->UpdateConstant(m_renderManager->directionalLightUniforms.data(), sizeof(ThreeDimension::DirectionalLightUniform) * m_renderManager->directionalLightUniforms.size(), m_renderManager->m_frameIndex);
		commandList->SetGraphicsRootConstantBufferView(0, m_renderManager->directionalLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
	}

	glm::mat4 inverseView = glm::inverse(Engine::GetCameraManager().GetViewMatrix());
	pushConstants = {
		.viewPosition = pushConstants.viewPosition = glm::vec3(
		inverseView[3].x,
		inverseView[3].y,
		inverseView[3].z
		),
		.meshletVisualization = m_renderManager->m_meshletVisualization ? 1 : 0,
		.activeDirectionalLight = static_cast<int>(m_renderManager->directionalLightUniforms.size()),
	};

	commandList->SetGraphicsRoot32BitConstants(1, sizeof(PushConstants) / 4, &pushConstants, 0);

	D3D12_GPU_DESCRIPTOR_HANDLE gBufferGpuHandle = m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	gBufferGpuHandle.ptr += static_cast<UINT64>(m_gBufferSrvHandle.second) * m_renderManager->m_srvDescriptorSize;
	commandList->SetGraphicsRootDescriptorTable(2, gBufferGpuHandle);
	if (m_renderManager->m_skyboxEnabled)
	{
		auto skyboxGpuHandle = m_renderManager->m_skyboxRenderContext->GetSkybox()->GetIrradianceMapSrv();
		commandList->SetGraphicsRootDescriptorTable(3, skyboxGpuHandle);
	}

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);

	// Skybox context will handle the transition back to COMMON state if skybox is enabled
	if (!m_renderManager->m_skyboxEnabled)
	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_renderManager->m_renderTarget->GetHDRRenderTarget().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_COMMON
		);
		commandList->ResourceBarrier(1, &barrier);
	}
}

void DXGlobalLightingContext::CleanUp()
{
}
