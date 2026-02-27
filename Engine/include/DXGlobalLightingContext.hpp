//Author: JEYOON YU
//Project: CubeEngine
//File: DXGlobalLightingContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include "DXPipeLine.hpp"
#include <glm/mat4x4.hpp>

class DXRenderManager;

class DXGlobalLightingContext : public IRenderContext
{
public:
	DXGlobalLightingContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXGlobalLightingContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;
private:
	DXRenderManager* m_renderManager;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	std::unique_ptr<DXPipeLine> m_pipeline;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, UINT> m_gBufferSrvHandle;

	// Push Constants for Global Lighting Pass
	struct alignas(16) PushConstants
	{
		glm::mat4 lightViewProjection;
		glm::vec3 viewPosition;
		int meshletVisualization;
		glm::vec3 shadowDirection;
		int activeDirectionalLight;
		int useShadow;
		float orthoSize;
	} pushConstants;
};
