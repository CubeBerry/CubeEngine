//Author: JEYOON YU
//Project: CubeEngine
//File: DXSkyboxRenderContext.cpp
#include "DXSkyboxRenderContext.hpp"

#include "DXRenderManager.hpp"
#include "DXCommandListWrapper.hpp"
#include "Engine.hpp"

void DXSkyboxRenderContext::Initialize()
{
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(2, CD3DX12_ROOT_PARAMETER1{});
	rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	CD3DX12_DESCRIPTOR_RANGE1 texSrvRange;
	texSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	rootParameters[1].InitAsDescriptorTable(1, &texSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	m_renderManager->CreateRootSignature(m_rootSignatureSkybox, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignatureSkybox->SetName(L"Skybox Root Signature"));

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	UINT sampleCount = m_renderManager->m_deferredRenderingEnabled ? 1 : m_renderManager->m_renderTarget->GetMSAASampleCount();
	UINT sampleQuality = m_renderManager->m_deferredRenderingEnabled ? 0 : m_renderManager->m_renderTarget->GetMSAAQualityLevel();

	std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	m_pipelineSkybox = DXPipeLineBuilder(m_renderManager->m_device, m_rootSignatureSkybox)
		.SetShaders("../Engine/shaders/hlsl/Skybox.vert.hlsl", "../Engine/shaders/hlsl/Skybox.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, true)
		.SetDepthStencil(true, false)
		.SetRenderTargets(rtvFormats)
		.SetSampleDesc(sampleCount, sampleQuality)
		.Build();
}

void DXSkyboxRenderContext::OnResize()
{
	m_pipelineSkybox.reset();
	Initialize();
}

void DXSkyboxRenderContext::Execute(ICommandListWrapper* commandListWrapper)
{
	if (!m_renderManager->m_skyboxEnabled || !m_skybox) return;

	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_renderManager->m_deferredRenderingEnabled ?
		m_renderManager->m_renderTarget->GetHDRRtvHeap()->GetCPUDescriptorHandleForHeapStart() :
		m_renderManager->m_renderTarget->GetMSAARtvHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_renderManager->m_renderTarget->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	commandList->SetPipelineState(m_pipelineSkybox->GetPipelineState().Get());
	commandList->SetGraphicsRootSignature(m_rootSignatureSkybox.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* ppHeapsSkybox[] = { m_renderManager->m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeapsSkybox), ppHeapsSkybox);

	glm::mat4 worldToNDC[2] = { Engine::GetCameraManager().GetViewMatrix(), Engine::GetCameraManager().GetProjectionMatrix() };
	commandList->SetGraphicsRoot32BitConstants(0, 32, &worldToNDC, 0);
	commandList->SetGraphicsRootDescriptorTable(1, m_skybox->GetCubemapSrv());

	D3D12_VERTEX_BUFFER_VIEW vbv = m_skyboxVertexBuffer->GetView();
	commandList->IASetVertexBuffers(0, 1, &vbv);

	commandList->DrawInstanced(36, 1, 0, 0);

	if (m_renderManager->m_deferredRenderingEnabled)
	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_renderManager->m_renderTarget->GetHDRRenderTarget().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_COMMON
		);
		commandList->ResourceBarrier(1, &barrier);
	}
}

void DXSkyboxRenderContext::CleanUp()
{
	DeleteSkybox();
	m_pipelineSkybox.reset();
	m_rootSignatureSkybox.Reset();
}

void DXSkyboxRenderContext::LoadSkybox(const std::filesystem::path& path)
{
	CreateSkyboxGeometry();

	m_renderManager->WaitForGPU();
	m_skybox = std::make_unique<DXSkybox>(m_renderManager->m_device, m_renderManager->m_commandQueue, m_renderManager->m_srvHeap);
	std::array<std::pair<CD3DX12_CPU_DESCRIPTOR_HANDLE, UINT>, 5> skyboxSrvHandles;
	auto [startHandle, startIndex] = m_renderManager->AllocateSrvHandles(5);
	for (int i = 0; i < 5; ++i)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE currentHandle(startHandle, i, m_renderManager->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		skyboxSrvHandles[i] = { currentHandle, startIndex + i };
	}
	auto deallocator = [this](UINT index)
		{
			m_renderManager->DeallocateSrvBlock(index, 1);
		};
	m_skybox->Initialize(path, skyboxSrvHandles, deallocator);
	m_renderManager->m_skyboxEnabled = true;
}

void DXSkyboxRenderContext::DeleteSkybox()
{
	m_skyboxVertexBuffer.reset();
	m_skybox.reset();
	m_renderManager->m_skyboxEnabled = false;
}

void DXSkyboxRenderContext::CreateSkyboxGeometry()
{
	float skyboxVertices[] = {
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
	};

	m_skyboxVertexBuffer = std::make_unique<DXVertexBuffer>(m_renderManager->m_device, m_renderManager->m_commandQueue, static_cast<UINT>(sizeof(float)) * 3, static_cast<UINT>(sizeof(float)) * 108, skyboxVertices);
}
