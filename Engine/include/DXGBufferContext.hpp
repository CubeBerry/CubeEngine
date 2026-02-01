//Author: JEYOON YU
//Project: CubeEngine
//File: DXGBufferContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include <d3d12.h>
#include <wrl.h>
#include <array>
#include <string>
#include "DXPipeLine.hpp"
#include "DXMeshPipeLine.hpp"

using Microsoft::WRL::ComPtr;

class DXRenderManager;

enum class GBufferType : size_t
{
	Albedo = 0,
	Normal,
	WorldPosition,
	Material,
	Count
};

struct GBufferData
{
	DXGI_FORMAT format;
	ComPtr<ID3D12Resource> resource;
	std::wstring name;
};

class DXGBufferContext : public IRenderContext
{
public:
	DXGBufferContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXGBufferContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;

	ID3D12DescriptorHeap* GetSRVHeap() const { return m_srvHeap.Get(); }
private:
	DXRenderManager* m_renderManager;

	ComPtr<ID3D12RootSignature> m_rootSignature3D;
#ifdef _DEBUG
	ComPtr<ID3D12RootSignature> m_rootSignature3DNormal;
#endif

	std::unique_ptr<DXPipeLine> m_pipeline3D;
	std::unique_ptr<DXMeshPipeLine> m_meshPipeline3D;

	// RTV Heap for G-Buffer
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	// SRV Heap for G-Buffer
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;

	std::array<GBufferData, static_cast<size_t>(GBufferType::Count)> m_gBuffers =
	{
		GBufferData{ DXGI_FORMAT_R8G8B8A8_UNORM, nullptr, L"G-Buffer Albedo" },
		GBufferData{ DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr, L"G-Buffer Normal" },
		GBufferData{ DXGI_FORMAT_R32G32B32A32_FLOAT, nullptr, L"G-Buffer World Position" },
		GBufferData{ DXGI_FORMAT_R8G8B8A8_UNORM, nullptr, L"G-Buffer Material" }
	};
};
