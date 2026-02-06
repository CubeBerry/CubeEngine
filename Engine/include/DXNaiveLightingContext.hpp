//Author: JEYOON YU
//Project: CubeEngine
//File: DXNaiveLightingContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include "DXPipeLine.hpp"
#include <glm/vec3.hpp>

class DXRenderManager;

class DXNaiveLightingContext : public IRenderContext
{
public:
	DXNaiveLightingContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXNaiveLightingContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;
private:
	DXRenderManager* m_renderManager;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	std::unique_ptr<DXPipeLine> m_pipeline;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, UINT> m_gBufferSrvHandle;

	// Push Constants for Lighting Pass
	struct alignas(16) PushConstants
	{
		glm::vec3 viewPosition;
		int meshletVisualization;
		int activeDirectionalLight;
		int activePointLight;
	} pushConstants;
};
