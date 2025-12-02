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

	D3D12_SET_PROGRAM_DESC& GetSetProgramDesc() { return m_setProgramDesc; }
private:
	ComPtr<ID3D12StateObject> m_stateObject;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12Resource> m_backingMemory;
	D3D12_SET_PROGRAM_DESC m_setProgramDesc = {};
};