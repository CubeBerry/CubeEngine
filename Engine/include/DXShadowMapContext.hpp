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

	void SetEnabled(const bool enabled) { m_enabled = enabled; }

	bool IsEnabled() const { return m_enabled; }
	UINT GetSrvIndex() const { return m_srvHandle.second; }
	glm::mat4 GetLightViewProjection() const { return m_lightViewProjection; }
	glm::vec3 GetShadowDirection() const { return glm::normalize(m_lightTarget - m_lightPosition); }
	//float GetShadowBias() const { return m_shadowBias; }
	float GetOrthoSize() const { return m_orthoSize; }

	void DrawImGui();
private:
	void CreateDepthTexture();
	glm::mat4 CreateLightViewProjection();
	void UpdateBlurWeights() const;

	DXRenderManager* m_renderManager;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	std::unique_ptr<DXPipeLine> m_pipeline;
	bool m_enabled{ true };

	ComPtr<ID3D12Resource> m_momentTexture; // Color (RTV, SRV)
	ComPtr<ID3D12Resource> m_depthTexture; // Depth (DSV)
	UINT64 m_width{ 2048 };
	UINT m_height{ 2048 };

	// Shadow Parameters
	float m_nearPlane{ 1.f }, m_farPlane{ 40.f };
	float m_orthoSize{ 15.f };
	glm::vec3 m_lightPosition{ -3.f, 2.f, 0.f };
	glm::vec3 m_lightTarget{ 0.f, 0.f, 0.f };
	//float m_shadowBias{ 0.005f };

	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_rtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, UINT> m_srvHandle;

	// Push Constants for Shadow Map Pass
	struct alignas(16) PushConstants
	{
		glm::mat4 decode;
		glm::mat4 localToNDC;
	} pushConstants;
	glm::mat4 m_lightViewProjection;

	// Convolution Blur
	int m_blurWidth{ 5 };
	struct alignas(16) BlurParams
	{
		glm::vec4 weights[101];
		int blurWidth;
		int isVertical;
	};

	ComPtr<ID3D12RootSignature> m_computeRootSignature;
	ComPtr<ID3D12PipelineState> m_computePipelineState;
	ComPtr<ID3D12Resource> m_blurredMomentTexture;
	ComPtr<ID3D12Resource> m_blurParamsBuffer[2];
	BlurParams* m_blurParamsMapped[2]{ nullptr, nullptr };

	UINT m_computeSrvBaseIndex{ 0 };
};
