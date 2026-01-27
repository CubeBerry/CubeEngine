//Author: JEYOON YU
//Project: CubeEngine
//File: DXLightingContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include "DXPipeLine.hpp"
#include "DXMeshPipeLine.hpp"

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

	ComPtr<ID3D12RootSignature> m_rootSignature3D;
#ifdef _DEBUG
	ComPtr<ID3D12RootSignature> m_rootSignature3DNormal;
#endif

	std::unique_ptr<DXPipeLine> m_pipeline3D;
	std::unique_ptr<DXPipeLine> m_pipeline3DLine;
	std::unique_ptr<DXMeshPipeLine> m_meshPipeline3D;
#ifdef _DEBUG
	std::unique_ptr<DXPipeLine> m_pipeline3DNormal;
#endif
};
