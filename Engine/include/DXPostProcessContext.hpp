//Author: JEYOON YU
//Project: CubeEngine
//File: DXPostProcessContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"
#include "FidelityFX.hpp"

class DXRenderManager;
class DXPipeLine;

class DXPostProcessContext : public IRenderContext
{
public:
	DXPostProcessContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXPostProcessContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;

	void UpdateScalePreset(const FidelityFX::UpscaleEffect& effect, const FfxFsr1QualityMode& mode, const FidelityFX::CASScalePreset& preset) const;

	FidelityFX* GetFidelityFX() const { return m_fidelityFX.get(); }
private:
	DXRenderManager* m_renderManager;
	std::unique_ptr<FidelityFX> m_fidelityFX;

	// Tone Mapping
	ComPtr<ID3D12RootSignature> m_rootSignature;
	std::unique_ptr<DXPipeLine> m_pipeline;
};
