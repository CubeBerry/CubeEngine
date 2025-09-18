//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderTarget.hpp
#pragma once
#include <directx/d3dx12.h>
#include <wrl.h>

#include "SDL3/SDL.h"

using Microsoft::WRL::ComPtr;

// @TODO Class Name should be changed!
class DXRenderTarget
{
public:
	DXRenderTarget(
		const ComPtr<ID3D12Device>& device,
		SDL_Window* window,
		int width, int height
	);
	~DXRenderTarget() = default;

	DXRenderTarget(const DXRenderTarget&) = delete;
	DXRenderTarget& operator=(const DXRenderTarget&) = delete;
	DXRenderTarget(const DXRenderTarget&&) = delete;
	DXRenderTarget& operator=(const DXRenderTarget&&) = delete;

	// MSAA
	UINT GetMSAASampleCount() const { return m_msaaSampleCount; }
	UINT GetMSAAQualityLevel() const { return m_msaaQualityLevel; }
	ComPtr<ID3D12Resource> GetMSAARenderTarget() const { return m_msaaRenderTarget; }
	ComPtr<ID3D12DescriptorHeap> GetMSAARtvHeap() const { return m_msaaRtvHeap; }

	// Depth
	ComPtr<ID3D12Resource> GetDepthStencil() const { return m_depthStencil; }
	ComPtr<ID3D12DescriptorHeap> GetDsvHeap() const { return m_dsvHeap; }
private:
	// Common
	ComPtr<ID3D12Device> m_device;
	SDL_Window* m_window;

	// MSAA
	void CreateColorResources(int width, int height);

	UINT m_msaaSampleCount{ 4 };
	UINT m_msaaQualityLevel;
	ComPtr<ID3D12Resource> m_msaaRenderTarget;
	ComPtr<ID3D12DescriptorHeap> m_msaaRtvHeap;

	// Depth
	void CreateDepthBuffer(int width, int height);

	ComPtr<ID3D12Resource> m_depthStencil;
	// dsv = Depth Stencil View
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
};
