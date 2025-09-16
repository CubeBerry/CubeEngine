//Author: JEYOON YU
//Project: CubeEngine
//File: FidelityFX.hpp
#pragma once
#include "Kits/FidelityFX/api/include/dx12/ffx_api_dx12.hpp"

#include <directx/d3dx12.h>

class DXRenderTarget;

using Microsoft::WRL::ComPtr;

// https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK
class FidelityFX
{
public:
	FidelityFX() = default;
	FidelityFX(const FidelityFX&) = delete;
	FidelityFX& operator=(const FidelityFX&) = delete;
	FidelityFX(const FidelityFX&&) = delete;
	FidelityFX& operator=(const FidelityFX&&) = delete;

	~FidelityFX() = default;
private:
	void CreateContext(const ComPtr<ID3D12Device>& device);
	void Execute(
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12DescriptorHeap>& srvHeap,
		const std::unique_ptr<DXRenderTarget>& dxRenderTarget,
		const ComPtr<ID3D12Resource>& renderTarget
	);
	void QueryVersion(const ComPtr<ID3D12Device>& device);

	std::vector<uint64_t> m_fsrVersionIds;
	int32_t m_fsrVersionIndex{ 0 };
	bool m_overrideVersion{ false };
	std::vector<const char*> m_fsrVersionNames;
	ffx::Context m_upscalingContext{ nullptr };

	uint64_t m_currentUpscaleContextVersionId{ 0 };
	const char* m_currentUpscaleContextVersionName{ nullptr };

	ComPtr<ID3D12Resource> m_postProcessTexture;
};
