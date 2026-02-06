//Author: JEYOON YU
//Project: CubeEngine
//File: DXLocalLightingContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include "DXPipeLine.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class DXRenderManager;
class DXVertexBuffer;
class DXIndexBuffer;

class DXLocalLightingContext : public IRenderContext
{
public:
	DXLocalLightingContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXLocalLightingContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;
private:
	DXRenderManager* m_renderManager;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	std::unique_ptr<DXPipeLine> m_pipeline;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, UINT> m_gBufferSrvHandle;

	// Push Constants for Local Lighting Pass
	struct alignas(16) PushConstants
	{
		glm::mat4 viewProjection;
		glm::vec3 viewPosition;
		float intensity;
		glm::vec2 screenSize;
	};

	void CreateUnitSphere();
	std::unique_ptr<DXVertexBuffer> m_unitSphereVertexBuffer;
	std::unique_ptr<DXIndexBuffer> m_unitSphereIndexBuffer;
	UINT m_sphereIndexCount;
};
