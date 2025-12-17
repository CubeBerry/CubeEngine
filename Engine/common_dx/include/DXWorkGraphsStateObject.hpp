//Author: JEYOON YU
//Project: CubeEngine
//File: DXWorkGraphsStateObject.hpp
#pragma once
#include <d3dx12/d3dx12.h>

#include <filesystem>

using Microsoft::WRL::ComPtr;

// https://gpuopen.com/learn/gpu-work-graphs/gpu-work-graphs-intro/
class DXWorkGraphsStateObject
{
public:
	DXWorkGraphsStateObject(
		const ComPtr<ID3D12Device14>& device,
		//const ComPtr<ID3D12RootSignature>& rootSignature,
		const std::filesystem::path& nodePath,
		const wchar_t* workGraphName
	);
#if USE_PREVIEW_SDK
	DXWorkGraphsStateObject(
		const ComPtr<ID3D12Device14>& device,
		//const ComPtr<ID3D12RootSignature>& rootSignature,
		const std::filesystem::path& nodePath,
		const std::filesystem::path& pixelPath,
		const DXGI_FORMAT& depthFormat,
		const DXGI_FORMAT& renderTargetFormat,
		const wchar_t* workGraphName
	);
#endif
	~DXWorkGraphsStateObject() = default;

	D3D12_SET_PROGRAM_DESC GetProgramDesc() const;
	ID3D12RootSignature* GetGlobalRootSignature() const { return m_rootSignature.Get(); }
	//UINT GetWorkGraphIndex() const { return m_workGraphIndex; }
	UINT GetEntrypointIndex(LPCWSTR nodeName);
	//ID3D12StateObject* GetStateObject() const { return m_stateObject.Get(); }
private:
	const wchar_t* m_workGraphName = L"WorkGraphsTutorial";
	UINT m_workGraphIndex{ 0 };

	ComPtr<ID3D12StateObject> m_stateObject;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12Resource> m_backingMemory;
	ComPtr<ID3D12WorkGraphProperties> m_workGraphProperties;
	ComPtr<ID3D12StateObjectProperties1> m_stateObjectProperties;
	D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS m_memoryRequirements = {};
};