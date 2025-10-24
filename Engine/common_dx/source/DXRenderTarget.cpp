//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderTarget.cpp
#include "DXRenderTarget.hpp"
#include "DXHelper.hpp"

DXRenderTarget::DXRenderTarget(
	const ComPtr<ID3D12Device>& device,
	SDL_Window* window,
	int width, int height
	) : m_device(device), m_window(window)
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	DXHelper::ThrowIfFailed(m_dsvHeap->SetName(L"Depth/Stencil View Heap"));

	CreateColorResources(width, height);
	CreateDepthBuffer(width, height);
}

void DXRenderTarget::CreateColorResources(int width, int height)
{
	// Check MSAA Support
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels = {};
	msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msQualityLevels.SampleCount = m_msaaSampleCount;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

	DXHelper::ThrowIfFailed(m_device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)
	));

	if (msQualityLevels.NumQualityLevels == 0)
	{
		throw std::runtime_error("MSAA sample count not supported");
	}
	m_msaaQualityLevel = msQualityLevels.NumQualityLevels - 1;

	// MSAA
	D3D12_DESCRIPTOR_HEAP_DESC msaaRtvHeapDesc = {};
	msaaRtvHeapDesc.NumDescriptors = 1;
	msaaRtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	msaaRtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&msaaRtvHeapDesc, IID_PPV_ARGS(&m_msaaRtvHeap)));

	D3D12_RESOURCE_DESC msaaRenderTargetDesc = {};
	msaaRenderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	msaaRenderTargetDesc.Width = width;
	msaaRenderTargetDesc.Height = height;
	msaaRenderTargetDesc.DepthOrArraySize = 1;
	msaaRenderTargetDesc.MipLevels = 1;
	msaaRenderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaRenderTargetDesc.SampleDesc.Count = m_msaaSampleCount;
	msaaRenderTargetDesc.SampleDesc.Quality = m_msaaQualityLevel;
	msaaRenderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE msaaClearValue = {};
	msaaClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaClearValue.Color[0] = 0.f;
	msaaClearValue.Color[1] = 0.f;
	msaaClearValue.Color[2] = 0.f;
	msaaClearValue.Color[3] = 1.f;

	// heapProps(D3D12_HEAP_TYPE_DEFAULT)
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&msaaRenderTargetDesc,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		&msaaClearValue,
		IID_PPV_ARGS(&m_msaaRenderTarget)
	));
	m_msaaRenderTarget->SetName(L"MSAA Render Target View");

	m_device->CreateRenderTargetView(m_msaaRenderTarget.Get(), nullptr, m_msaaRtvHeap->GetCPUDescriptorHandleForHeapStart());
}

// Depth
void DXRenderTarget::CreateDepthBuffer(int width, int height)
{
	// Create Depth Stencil Buffer Resource
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = static_cast<UINT64>(width);
	depthStencilDesc.Height = static_cast<UINT>(height);
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	// @TODO Use ID3D12Device::CheckFeatureSupport to find supported format
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.SampleDesc.Count = m_msaaSampleCount;
	depthStencilDesc.SampleDesc.Quality = m_msaaQualityLevel;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_depthStencil)
	));
	DXHelper::ThrowIfFailed(m_depthStencil->SetName(L"Depth/Stencil View"));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}
