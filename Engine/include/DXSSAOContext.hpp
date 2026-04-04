//Author: JEYOON YU
//Project: CubeEngine
//File: DXSSAOContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include "DXPipeLine.hpp"
#include <glm/mat4x4.hpp>

class DXRenderManager;

class DXSSAOContext : public IRenderContext
{
public:
	DXSSAOContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXSSAOContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;

	UINT GetBlurredAOSrvIndex() const { return m_blurSrvHandle.second; }

	void DrawImGui();
private:
	DXRenderManager* m_renderManager;

	// SSAO Pass Resource
	ComPtr<ID3D12Resource> m_ssaoTexture; // R8_UNORM
	ComPtr<ID3D12DescriptorHeap> m_ssaoRtvHeap;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, UINT> m_ssaoSrvHandle;
	ComPtr<ID3D12RootSignature> m_ssaoRootSignature;
	std::unique_ptr<DXPipeLine> m_ssaoPipeline;

	// Blur Pass Resource
	ComPtr<ID3D12Resource> m_blurTexture; // R8_UNORM
	ComPtr<ID3D12DescriptorHeap> m_blurRtvHeap;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, UINT> m_blurSrvHandle;
	ComPtr<ID3D12RootSignature> m_blurRootSignature;
	std::unique_ptr<DXPipeLine> m_blurPipeline;

	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, UINT> m_gBufferSrvHandle;

	// Push Constants for SSAO
	struct alignas(16) PushConstants
	{
		glm::mat4 view;
		glm::mat4 projection;
		float radius;
		float scale;
		float contrast;
		int numSamples;
		float delta;
		float screenWidth;
		float screenHeight;
	} pushConstants;

	// ImGui Parameters
	float m_radius = 1.0f;
	float m_scale = 1.0f;
	float m_contrast = 1.0f;
	int m_numSamples = 15;
	float m_delta = 0.001f;
	bool m_enabled = true;

	// Helper Functions
	void CreateSSAOResources();
	void CreateBlurResources();
};
