//Author: JEYOON YU
//Project: CubeEngine
//File: FidelityFX.hpp
#pragma once
#include "FidelityFX/host/ffx_cas.h"
// @TODO FidelityFX SDK 2.0
//#include "Kits/FidelityFX/api/include/dx12/ffx_api_dx12.hpp"

#include <wrl.h>
#include <directx/d3dx12.h>

class DXRenderTarget;

using Microsoft::WRL::ComPtr;

// FidelityFX SDK 1.1.4
class FidelityFX
{
public:
	enum class CASScalePreset
	{
		UltraQuality = 0,  // 1.3f
		Quality,           // 1.5f
		Balanced,          // 1.7f
		Performance,       // 2.f
		UltraPerformance,  // 3.f
		Custom             // 1.f - 3.f range
	};
public:
	FidelityFX() = default;
	FidelityFX(const FidelityFX&) = delete;
	FidelityFX& operator=(const FidelityFX&) = delete;
	FidelityFX(const FidelityFX&&) = delete;
	FidelityFX& operator=(const FidelityFX&&) = delete;

	~FidelityFX();

	void CreateCasContext(
		const ComPtr<ID3D12Device>& device,
		int displayWidth, int displayHeight
	);
	void OnResize(
		const ComPtr<ID3D12Device>& device,
		int displayWidth, int displayHeight
	);
	void UpdateScalePreset(
		const ComPtr<ID3D12Device>& device,
		bool enableUpscaling,
		CASScalePreset preset);
	void Execute(
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12Resource>& inputRenderTarget,
		const ComPtr<ID3D12Resource>& outputRenderTarget
	);

	bool GetEnableUpscaling() const { return m_enableUpscaling; }
	CASScalePreset GetScalePreset() const { return m_scalePreset; }
	uint32_t GetRenderWidth() const { return m_renderWidth; }
	uint32_t GetRenderHeight() const { return m_renderHeight; }
private:
	// FidelityFX SDK 1.1.4
	FfxCasContextDescription m_initializationParameters{ 0 };
	FfxCasContext m_casContext;
	ComPtr<ID3D12Resource> m_postProcessTexture;

	float m_sharpness{ 0.8f };
	bool m_enableUpscaling{ false };
	CASScalePreset m_scalePreset{ CASScalePreset::UltraQuality };
	float m_upscaleRatio{ 1.3f };

	int m_displayWidth, m_displayHeight;
	uint32_t m_renderWidth, m_renderHeight;

	// @TODO FidelityFX SDK 2.0
	//std::vector<uint64_t> m_fsrVersionIds;
	//int32_t m_fsrVersionIndex{ 0 };
	//bool m_overrideVersion{ false };
	//std::vector<const char*> m_fsrVersionNames;
	//ffx::Context m_upscalingContext{ nullptr };

	//uint64_t m_currentUpscaleContextVersionId{ 0 };
	//const char* m_currentUpscaleContextVersionName{ nullptr };

	//ComPtr<ID3D12Resource> m_postProcessTexture;
};

// @TODO FidelityFX SDK 2.0
// https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK
//class FidelityFX
//{
//public:
//	FidelityFX() = default;
//	FidelityFX(const FidelityFX&) = delete;
//	FidelityFX& operator=(const FidelityFX&) = delete;
//	FidelityFX(const FidelityFX&&) = delete;
//	FidelityFX& operator=(const FidelityFX&&) = delete;
//
//	~FidelityFX() = default;
//private:
//	void CreateContext(const ComPtr<ID3D12Device>& device);
//	void Execute(
//		const ComPtr<ID3D12GraphicsCommandList>& commandList,
//		const ComPtr<ID3D12DescriptorHeap>& srvHeap,
//		const std::unique_ptr<DXRenderTarget>& dxRenderTarget,
//		const ComPtr<ID3D12Resource>& renderTarget
//	);
//	void QueryVersion(const ComPtr<ID3D12Device>& device);
//
//	std::vector<uint64_t> m_fsrVersionIds;
//	int32_t m_fsrVersionIndex{ 0 };
//	bool m_overrideVersion{ false };
//	std::vector<const char*> m_fsrVersionNames;
//	ffx::Context m_upscalingContext{ nullptr };
//
//	uint64_t m_currentUpscaleContextVersionId{ 0 };
//	const char* m_currentUpscaleContextVersionName{ nullptr };
//
//	ComPtr<ID3D12Resource> m_postProcessTexture;
//};
