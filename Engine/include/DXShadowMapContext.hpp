//Author: JEYOON YU
//Project: CubeEngine
//File: DXShadowMapContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

class DXRenderManager;

class DXShadowMapContext : public IRenderContext
{
public:
	DXShadowMapContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXShadowMapContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;
private:
	DXRenderManager* m_renderManager;
};
