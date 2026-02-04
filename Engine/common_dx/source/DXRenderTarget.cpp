//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderTarget.cpp
#include "DXRenderTarget.hpp"
#include "DXHelper.hpp"

DXRenderTarget::DXRenderTarget(
	const ComPtr<ID3D12Device>& device,
	SDL_Window* window,
	int width, int height,
	bool deferred
	) : m_device(device), m_window(window)
{
	CreateHDRRenderTarget(width, height);
	CreateMSAARenderTarget(width, height);
	CreateDepthBuffer(width, height, deferred);
}

void DXRenderTarget::CreateHDRRenderTarget(int width, int height)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_hdrRtvHeap)));
	m_hdrRtvHeap->SetName(L"HDR RTV Heap");

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = textureDesc.Format;
	clearValue.Color[0] = 0.f;
	clearValue.Color[1] = 0.f;
	clearValue.Color[2] = 0.f;
	clearValue.Color[3] = 0.f;

	// heapProps(D3D12_HEAP_TYPE_DEFAULT)
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&clearValue,
		IID_PPV_ARGS(&m_hdrRenderTarget)
	));
	m_hdrRenderTarget->SetName(L"HDR Render Target View");

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	m_device->CreateRenderTargetView(m_hdrRenderTarget.Get(), &rtvDesc, m_hdrRtvHeap->GetCPUDescriptorHandleForHeapStart());
}

void DXRenderTarget::CreateMSAARenderTarget(int width, int height)
{
	// Check MSAA Support
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels = {};
	msQualityLevels.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
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
	m_msaaRtvHeap->SetName(L"MSAA RTV Heap");

	D3D12_RESOURCE_DESC msaaRenderTargetDesc = {};
	msaaRenderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	msaaRenderTargetDesc.Width = width;
	msaaRenderTargetDesc.Height = height;
	msaaRenderTargetDesc.DepthOrArraySize = 1;
	msaaRenderTargetDesc.MipLevels = 1;
	msaaRenderTargetDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	msaaRenderTargetDesc.SampleDesc.Count = m_msaaSampleCount;
	msaaRenderTargetDesc.SampleDesc.Quality = m_msaaQualityLevel;
	msaaRenderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE msaaClearValue = {};
	msaaClearValue.Format = msaaRenderTargetDesc.Format;
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
void DXRenderTarget::CreateDepthBuffer(int width, int height, bool deferred)
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	DXHelper::ThrowIfFailed(m_dsvHeap->SetName(L"Depth/Stencil View Heap"));

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
	depthStencilDesc.SampleDesc.Count = deferred ? 1 : m_msaaSampleCount;
	depthStencilDesc.SampleDesc.Quality = deferred ? 0 : m_msaaQualityLevel;
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
	dsvDesc.ViewDimension = deferred ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void DXRenderTarget::CreateSRV(D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle) const
{
	// Create SRV for HDR Render Target
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;

	m_device->CreateShaderResourceView(m_hdrRenderTarget.Get(), &srvDesc, srvCpuHandle);
}
