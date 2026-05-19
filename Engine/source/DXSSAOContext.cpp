//Author: JEYOON YU
//Project: CubeEngine
//File: DXSSAOContext.cpp
#include "DXSSAOContext.hpp"

#include <random>

#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
#include "DXSkyboxRenderContext.hpp"
#include "Engine.hpp"

void DXSSAOContext::Initialize()
{
	CreateSSAOResources();
	CreateBlurResources();

	m_gBufferSrvHandle = m_renderManager->AllocateSrvHandles(4);
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_gBufferSrvHandle.first;
	D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = m_renderManager->GetGBufferContext()->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	m_renderManager->m_device->CopyDescriptorsSimple(4, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// SSAO Root Signature & Pipeline
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(2);
	rootParameters[0].InitAsConstants(sizeof(PushConstants) / 4, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);;
	CD3DX12_DESCRIPTOR_RANGE1 gBufferRange;
	gBufferRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 1);
	rootParameters[1].InitAsDescriptorTable(1, &gBufferRange, D3D12_SHADER_VISIBILITY_PIXEL);

	m_renderManager->CreateRootSignature(m_ssaoRootSignature, rootParameters);
	DXHelper::ThrowIfFailed(m_ssaoRootSignature->SetName(L"SSAO Root Signature"));

	std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R8_UNORM };
	m_ssaoPipeline = DXPipeLineBuilder(m_renderManager->m_device, m_ssaoRootSignature)
		.SetShaders("../Engine/shaders/hlsl/SSAO.vert.hlsl", "../Engine/shaders/hlsl/SSAO.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{})
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false)
		.SetDepthStencil(false, false)
		.SetRenderTargets(rtvFormats)
		.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.Build();

	// Blur Root Signature & Pipeline
	rootParameters.resize(3);
	rootParameters[0].InitAsConstants(sizeof(PushConstants) / 4, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);;
	rootParameters[1].InitAsDescriptorTable(1, &gBufferRange, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_DESCRIPTOR_RANGE1 ssaoRange;
	ssaoRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 1);
	rootParameters[2].InitAsDescriptorTable(1, &ssaoRange, D3D12_SHADER_VISIBILITY_PIXEL);

	m_renderManager->CreateRootSignature(m_blurRootSignature, rootParameters);
	DXHelper::ThrowIfFailed(m_blurRootSignature->SetName(L"SSAO Blur Root Signature"));

	m_blurPipeline = DXPipeLineBuilder(m_renderManager->m_device, m_blurRootSignature)
		.SetShaders("../Engine/shaders/hlsl/SSAO.vert.hlsl", "../Engine/shaders/hlsl/SSAOBlur.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{})
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false)
		.SetDepthStencil(false, false)
		.SetRenderTargets(rtvFormats)
		.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.Build();
}

void DXSSAOContext::OnResize()
{
	m_ssaoTexture.Reset();
	m_ssaoRtvHeap.Reset();

	m_blurIntermediateTexture.Reset();
	m_blurIntermediateRtvHeap.Reset();

	m_blurTexture.Reset();
	m_blurRtvHeap.Reset();

	m_renderManager->DeallocateSrvBlock(m_ssaoSrvHandle.second, 1);
	m_renderManager->DeallocateSrvBlock(m_blurIntermediateSrvHandle.second, 1);
	m_renderManager->DeallocateSrvBlock(m_blurSrvHandle.second, 1);

	CreateSSAOResources();
	CreateBlurResources();

	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_gBufferSrvHandle.first;
	D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = m_renderManager->GetGBufferContext()->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	m_renderManager->m_device->CopyDescriptorsSimple(4, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DXSSAOContext::Execute(ICommandListWrapper* commandListWrapper)
{
	if (!m_enabled) return;

	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	int width = m_renderManager->GetPostProcessContext()->GetFidelityFX()->GetRenderWidth();
	int height = m_renderManager->GetPostProcessContext()->GetFidelityFX()->GetRenderHeight();

	// Set Viewport and Scissor Rect
	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(width), static_cast<FLOAT>(height), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, width, height };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	// Update Push Constants
	pushConstants.view = Engine::GetCameraManager().GetViewMatrix();
	pushConstants.projection = Engine::GetCameraManager().GetProjectionMatrix();
	pushConstants.radius = m_radius;
	pushConstants.scale = m_scale;
	pushConstants.contrast = m_contrast;
	pushConstants.numSamples = m_numSamples;
	pushConstants.delta = m_delta;
	pushConstants.screenWidth = static_cast<float>(width);
	pushConstants.screenHeight = static_cast<float>(height);

	// Set Descriptor Heaps
	ID3D12DescriptorHeap* ppHeaps[] = { m_renderManager->m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(1, ppHeaps);

	// SSAO Pass
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_ssaoTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	commandList->SetPipelineState(m_ssaoPipeline->GetPipelineState().Get());
	commandList->SetGraphicsRootSignature(m_ssaoRootSignature.Get());

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_ssaoRtvHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	commandList->SetGraphicsRoot32BitConstants(0, sizeof(PushConstants) / 4, &pushConstants, 0);
	D3D12_GPU_DESCRIPTOR_HANDLE gBufferGpuHandle = m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	gBufferGpuHandle.ptr += static_cast<UINT64>(m_gBufferSrvHandle.second) * m_renderManager->m_srvDescriptorSize;
	commandList->SetGraphicsRootDescriptorTable(1, gBufferGpuHandle);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_ssaoTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);

	// Bilateral Blur Pass (Horizontal)
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_blurIntermediateTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	commandList->SetPipelineState(m_blurPipeline->GetPipelineState().Get());
	commandList->SetGraphicsRootSignature(m_blurRootSignature.Get());

	rtvHandle = m_blurIntermediateRtvHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	pushConstants.blurDirection = glm::ivec2(1, 0);
	commandList->SetGraphicsRoot32BitConstants(0, sizeof(PushConstants) / 4, &pushConstants, 0);
	commandList->SetGraphicsRootDescriptorTable(1, gBufferGpuHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE ssaoSrvGpuHandle = m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	ssaoSrvGpuHandle.ptr += static_cast<UINT64>(m_ssaoSrvHandle.second) * m_renderManager->m_srvDescriptorSize;
	commandList->SetGraphicsRootDescriptorTable(2, ssaoSrvGpuHandle);

	commandList->DrawInstanced(3, 1, 0, 0);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_blurIntermediateTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);

	// Bilateral Blur Pass (Vertical)
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_blurTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	commandList->SetPipelineState(m_blurPipeline->GetPipelineState().Get());
	commandList->SetGraphicsRootSignature(m_blurRootSignature.Get());

	rtvHandle = m_blurRtvHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	pushConstants.blurDirection = glm::ivec2(0, 1);
	commandList->SetGraphicsRoot32BitConstants(0, sizeof(PushConstants) / 4, &pushConstants, 0);
	commandList->SetGraphicsRootDescriptorTable(1, gBufferGpuHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE intermediateSrvGpuHandle = m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	intermediateSrvGpuHandle.ptr += static_cast<UINT64>(m_blurIntermediateSrvHandle.second) * m_renderManager->m_srvDescriptorSize;
	commandList->SetGraphicsRootDescriptorTable(2, intermediateSrvGpuHandle);

	commandList->DrawInstanced(3, 1, 0, 0);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_blurTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);
}

void DXSSAOContext::CleanUp()
{
}

void DXSSAOContext::CreateSSAOResources()
{
	const int width = m_renderManager->GetPostProcessContext()->GetFidelityFX()->GetRenderWidth();
	const int height = m_renderManager->GetPostProcessContext()->GetFidelityFX()->GetRenderHeight();

	// 1. Create SSAO Render Target
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Width = static_cast<UINT64>(width);
	textureDesc.Height = static_cast<UINT>(height);
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_R8_UNORM;
	clearValue.Color[0] = 1.0f;
	clearValue.Color[1] = 1.0f;
	clearValue.Color[2] = 1.0f;
	clearValue.Color[3] = 1.0f;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&m_ssaoTexture)
	));

	// 2. Create RTV Heap for SSAO Render Target
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_ssaoRtvHeap)));
	DXHelper::ThrowIfFailed(m_ssaoRtvHeap->SetName(L"SSAO Render Target View Heap"));

	// 3. Create RTV for SSAO Render Target
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_ssaoRtvHeap->GetCPUDescriptorHandleForHeapStart());
	m_renderManager->m_device->CreateRenderTargetView(m_ssaoTexture.Get(), nullptr, rtvHandle);

	// Create SRV in Main SRV Heap
	m_ssaoSrvHandle = m_renderManager->AllocateSrvHandles(1);
	m_renderManager->m_device->CreateShaderResourceView(m_ssaoTexture.Get(), nullptr, m_ssaoSrvHandle.first);
}

void DXSSAOContext::CreateBlurResources()
{
	const int width = m_renderManager->GetPostProcessContext()->GetFidelityFX()->GetRenderWidth();
	const int height = m_renderManager->GetPostProcessContext()->GetFidelityFX()->GetRenderHeight();

	// 1. Create SSAO Render Target
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Width = static_cast<UINT64>(width);
	textureDesc.Height = static_cast<UINT>(height);
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_R8_UNORM;
	clearValue.Color[0] = 1.0f;
	clearValue.Color[1] = 1.0f;
	clearValue.Color[2] = 1.0f;
	clearValue.Color[3] = 1.0f;

	// 2. Create Intermediate Blur Render Target (Horizontal Pass)
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&m_blurIntermediateTexture)
	));

	// Create RTV Heap for SSAO Render Target
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_blurIntermediateRtvHeap)));
	DXHelper::ThrowIfFailed(m_blurIntermediateRtvHeap->SetName(L"SSAO Intermediate Blur Render Target View Heap"));

	// Create RTV for SSAO Render Target
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_blurIntermediateRtvHeap->GetCPUDescriptorHandleForHeapStart());
	m_renderManager->m_device->CreateRenderTargetView(m_blurIntermediateTexture.Get(), nullptr, rtvHandle);

	// Create SRV in Main SRV Heap
	m_blurIntermediateSrvHandle = m_renderManager->AllocateSrvHandles(1);
	m_renderManager->m_device->CreateShaderResourceView(m_blurIntermediateTexture.Get(), nullptr, m_blurIntermediateSrvHandle.first);

	// 3. Create Final Blur Render Target (Vertical Pass)
	heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&m_blurTexture)
	));

	// Create RTV Heap for SSAO Render Target
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_blurRtvHeap)));
	DXHelper::ThrowIfFailed(m_blurRtvHeap->SetName(L"SSAO Final Blur Render Target View Heap"));

	// Create RTV for SSAO Render Target
	rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_blurRtvHeap->GetCPUDescriptorHandleForHeapStart());
	m_renderManager->m_device->CreateRenderTargetView(m_blurTexture.Get(), nullptr, rtvHandle);

	// Create SRV in Main SRV Heap
	m_blurSrvHandle = m_renderManager->AllocateSrvHandles(1);
	m_renderManager->m_device->CreateShaderResourceView(m_blurTexture.Get(), nullptr, m_blurSrvHandle.first);
}

void DXSSAOContext::DrawImGui()
{
	if (ImGui::CollapsingHeader("Ambient Occlusion Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (m_enabled)
		{
			ImGui::SliderFloat("Radius", &m_radius, 0.01f, 2.0f, "%.3f");
			ImGui::SliderFloat("Scale", &m_scale, 0.1f, 5.0f, "%.2f");
			ImGui::SliderFloat("Contrast", &m_contrast, 0.1f, 5.0f, "%.2f");
			ImGui::SliderInt("Samples", &m_numSamples, 1, 32);
			ImGui::DragFloat("Depth Delta", &m_delta, 0.0001f, 0.0f, 0.05f, "%.4f");
		}
	}
}
