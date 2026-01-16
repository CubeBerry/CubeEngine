//Author: JEYOON YU
//Project: CubeEngine
//File: DXForwardRenderContext.hpp
#pragma once
#include "Interface/IRenderContext.hpp"

#include <d3d12.h>
#include <wrl\client.h>

#include "DXPipeLine.hpp"
#include "DXMeshPipeLine.hpp"

using Microsoft::WRL::ComPtr;

class DXRenderManager;

class DXForwardRenderContext : public IRenderContext
{
public:
	DXForwardRenderContext(DXRenderManager* renderManager) : m_renderManager(renderManager) {}
	~DXForwardRenderContext() override = default;

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
public:
	class DXCommandListWrapper : public ICommandListWrapper
	{
	public:
		DXCommandListWrapper(ID3D12GraphicsCommandList10* commandList) : m_commandList(commandList) {}
		void* GetNativeHandle() const override { return m_commandList; }
		ID3D12GraphicsCommandList10* GetDXCommandList() const { return m_commandList; }
	private:
		ID3D12GraphicsCommandList10* m_commandList;
	};
};
