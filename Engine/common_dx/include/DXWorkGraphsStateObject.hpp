//Author: JEYOON YU
//Project: CubeEngine
//File: DXWorkGraphsStateObject.hpp
#pragma once
#include <directx/d3dx12.h>

#include <filesystem>

using Microsoft::WRL::ComPtr;

// https://gpuopen.com/learn/gpu-work-graphs/gpu-work-graphs-intro/
class DXWorkGraphsStateObject
{
public:
	DXWorkGraphsStateObject(
		const ComPtr<ID3D12Device14>& device,
		//const ComPtr<ID3D12RootSignature>& rootSignature,
		const std::filesystem::path& nodePath
	);
	~DXWorkGraphsStateObject() = default;

	//ComPtr<ID3D12PipelineState> GetPipelineState() const { return m_pipelineState; }
private:
	//ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12StateObject> m_stateObject;
	ComPtr<ID3D12RootSignature> m_rootSignature;
};