//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderTarget.hpp
#pragma once
#include <d3dx12/d3dx12.h>
#include <wrl.h>

#include "SDL3/SDL.h"

using Microsoft::WRL::ComPtr;

// @TODO Class Name should be changed!
// This class manages MSAA Render Target and Depth Buffer
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

	// Intermediate Render Target Resources
	ComPtr<ID3D12Resource> GetRenderTarget() const { return m_renderTarget; }
	ComPtr<ID3D12DescriptorHeap> GetRtvHeap() const { return m_rtvHeap; }

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

	// Intermediate Render Target Resources
	void CreateRenderTarget(int width, int height);

	ComPtr<ID3D12Resource> m_renderTarget;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

	// MSAA
	void CreateMSAARenderTarget(int width, int height);

	UINT m_msaaSampleCount{ 4 };
	UINT m_msaaQualityLevel;
	ComPtr<ID3D12Resource> m_msaaRenderTarget;
	ComPtr<ID3D12DescriptorHeap> m_msaaRtvHeap;

	// Depth
	void CreateDepthBuffer(int width, int height);

	// @TODO Move this to Forward Render Context later because G-Buffer Context has its own depth buffer
	ComPtr<ID3D12Resource> m_depthStencil;
	// dsv = Depth Stencil View
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
};
