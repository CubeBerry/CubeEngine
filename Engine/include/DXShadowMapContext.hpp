//Author: JEYOON YU
//Project: CubeEngine
//File: DXShadowMapContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include <d3dx12/d3dx12.h>
#include <glm/mat4x4.hpp>

#include "imgui.h"

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

	void SetEnabled(const bool enabled) { m_enabled = enabled; }

	bool IsEnabled() const { return m_enabled; }
	UINT GetSrvIndex() const { return m_srvHandle.second; }
	glm::mat4 GetLightViewProjection() const { return m_lightViewProjection; }

	void DrawImGui();
private:
	void CreateDepthTexture();
	glm::mat4 CreateLightViewProjection() const;

	DXRenderManager* m_renderManager;
	std::unique_ptr<DXPipeLine> m_pipeline;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	bool m_enabled{ true };

	ComPtr<ID3D12Resource> m_texture;
	UINT64 m_width{ 2048 };
	UINT m_height{ 2048 };

	// Shadow Parameters
	float m_nearPlane{ 1.f }, m_farPlane{ 40.f };
	float m_orthoSize{ 10.f };
	glm::vec3 m_lightPosition{ -3.f, 2.f, 0.f };
	glm::vec3 m_lightTarget{ 0.f, 0.f, 0.f };

	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	//ComPtr<ID3D12Resource> m_depthStencil;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, UINT> m_srvHandle;

	// Push Constants for Shadow Map Pass
	struct alignas(16) PushConstants
	{
		glm::mat4 decode;
		glm::mat4 localToNDC;
	} pushConstants;
	glm::mat4 m_lightViewProjection;
};
