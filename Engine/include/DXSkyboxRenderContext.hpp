//Author: JEYOON YU
//Project: CubeEngine
//File: DXSkyboxRenderContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"
#include "DXSkybox.hpp"

#include <filesystem>

class DXRenderManager;

class DXSkyboxRenderContext : public IRenderContext
{
public:
	DXSkyboxRenderContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXSkyboxRenderContext() override = default;

	void Initialize() override;
	void OnResize() override;
	void Execute(ICommandListWrapper* commandListWrapper) override;
	void CleanUp() override;

	void LoadSkybox(const std::filesystem::path& path);
	void DeleteSkybox();

	DXSkybox* GetSkybox() const { return m_skybox.get(); }
private:
	DXRenderManager* m_renderManager;

	std::unique_ptr<DXVertexBuffer> m_skyboxVertexBuffer;
	ComPtr<ID3D12RootSignature> m_rootSignatureSkybox;
	std::unique_ptr<DXPipeLine> m_pipelineSkybox;
	std::unique_ptr<DXSkybox> m_skybox;

	void CreateSkyboxGeometry();
};
