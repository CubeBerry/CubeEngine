//Author: JEYOON YU
//Project: CubeEngine
//File: DXShadowMapContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include <d3dx12/d3dx12.h>
#include <glm/mat4x4.hpp>

using Microsoft::WRL::ComPtr;

class DXRenderManager;
class DXPipeLine;

class DXShadowMapContext : public IRenderContext
{
public:
	DXShadowMapContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXShadowMapContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;
private:
	void CreateDepthTexture();
	glm::mat4 CreateLightViewProjection(const glm::mat4& model) const;

	DXRenderManager* m_renderManager;
	std::unique_ptr<DXPipeLine> m_pipeline;
	ComPtr<ID3D12RootSignature> m_rootSignature;

	ComPtr<ID3D12Resource> m_texture;
	UINT64 m_width{ 1024 };
	UINT m_height{ 1024 };

	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	//ComPtr<ID3D12Resource> m_depthStencil;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, UINT> m_srvHandle;

	// Push Constants for Shadow Map Pass
	struct alignas(16) PushConstants
	{
		glm::mat4 localToNDC;
	} pushConstants;
};
