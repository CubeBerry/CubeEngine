//Author: JEYOON YU
//Project: CubeEngine
//File: DXGBufferContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include <d3d12.h>
#include <wrl.h>
//#include "DXPipeLine.hpp"
//#include "DXMeshPipeLine.hpp"

using Microsoft::WRL::ComPtr;

class DXRenderManager;

class DXGBufferContext : public IRenderContext
{
public:
	DXGBufferContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXGBufferContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;
private:
	DXRenderManager* m_renderManager;

//	ComPtr<ID3D12RootSignature> m_rootSignature3D;
//#ifdef _DEBUG
//	ComPtr<ID3D12RootSignature> m_rootSignature3DNormal;
//#endif
//
//	std::unique_ptr<DXPipeLine> m_pipeline3D;
//	std::unique_ptr<DXPipeLine> m_pipeline3DLine;
//	std::unique_ptr<DXMeshPipeLine> m_meshPipeline3D;
//#ifdef _DEBUG
//	std::unique_ptr<DXPipeLine> m_pipeline3DNormal;
//#endif

	// RTV Heap for G-Buffer
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	// SRV Heap for G-Buffer
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;

	ComPtr<ID3D12Resource> m_gBufferAlbedo;
	ComPtr<ID3D12Resource> m_gBufferNormal;
	ComPtr<ID3D12Resource> m_gBufferWorldPosition;
	ComPtr<ID3D12Resource> m_gBufferMaterial;
};
