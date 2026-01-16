//Author: JEYOON YU
//Project: CubeEngine
//File: DX2DRenderContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include "DXPipeLine.hpp"

class DXRenderManager;

class DX2DRenderContext : public IRenderContext
{
public:
	DX2DRenderContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DX2DRenderContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;
private:
	DXRenderManager* m_renderManager;

	ComPtr<ID3D12RootSignature> m_rootSignature2D;
	std::unique_ptr<DXPipeLine> m_pipeline2D;
};
