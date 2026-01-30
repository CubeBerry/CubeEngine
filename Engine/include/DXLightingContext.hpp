//Author: JEYOON YU
//Project: CubeEngine
//File: DXLightingContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include "DXPipeLine.hpp"
#include <glm/vec3.hpp>

class DXRenderManager;

class DXLightingContext : public IRenderContext
{
public:
	DXLightingContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXLightingContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;
private:
	DXRenderManager* m_renderManager;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	std::unique_ptr<DXPipeLine> m_pipeline;
	// Copies G-Buffer SRV and IBL SRV from GBufferContext's m_srvHeap and DXRenderManager's m_srvHeap
	// @TODO Optimize by sharing descriptor heaps instead of copying or other methods (Think of a better way to refactor managing descriptor heaps)
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;

	// Push Constants for Lighting Pass
	struct alignas(16) PushConstants
	{
		glm::vec3 viewPosition;
		int activeDirectionalLight;
		int activePointLight;
	} pushConstants;
};
