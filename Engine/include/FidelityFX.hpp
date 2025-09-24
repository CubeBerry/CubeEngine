//Author: JEYOON YU
//Project: CubeEngine
//File: FidelityFX.hpp
#pragma once
// FidelityFX SDK 1.1.4
#include "FidelityFX/host/ffx_cas.h"
#include "FidelityFX/host/ffx_fsr1.h"
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
	enum class Effect
	{
		NONE,
		FSR1,
		CAS_SHARPEN_ONLY,
		CAS_UPSCALING
	};
	// CAS
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

	void InitializeBackend(const ComPtr<ID3D12Device>& device, int displayWidth, int displayHeight);
	// CAS
	void CreateCasContext();
	// FSR1
	void CreateFSR1Context();

	bool UpdatePreset(Effect effect, FfxFsr1QualityMode fsr1QualityMode, CASScalePreset casScalePreset);

	void OnResize(
		const ComPtr<ID3D12Device>& device,
		int displayWidth, int displayHeight
	);
	void Execute(
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12Resource>& input,
		const ComPtr<ID3D12Resource>& output
	);

	Effect GetCurrentEffect() const { return m_currentEffect; }
	bool GetEnableRCAS() const { return m_enableRCAS; }
	void SetEnableRCAS(bool enable) { m_enableRCAS = enable; }
	uint32_t GetRenderWidth() const { return m_renderWidth; }
	uint32_t GetRenderHeight() const { return m_renderHeight; }
	FfxFsr1QualityMode GetFSR1QualityMode() const { return m_fsr1QualityMode; }
	CASScalePreset GetScalePreset() const { return m_casScalePreset; }

	float m_sharpness{ 0.8f };
private:
	// FidelityFX SDK 1.1.4
	FfxInterface m_backendInterface;
	void* m_scratchBuffer{ nullptr };
	ComPtr<ID3D12Resource> m_postProcessTexture;

	Effect m_currentEffect{ Effect::NONE };
	bool m_enableRCAS{ false };
	int m_displayWidth, m_displayHeight;
	uint32_t m_renderWidth, m_renderHeight;

	// CAS
	FfxCasContextDescription m_casContextDesc{};
	FfxCasContext m_casContext;
	CASScalePreset m_casScalePreset{ CASScalePreset::UltraQuality };
	float m_casUpscaleRatio{ 1.3f };

	// FSR1
	FfxFsr1ContextDescription m_fsr1ContextDesc{};
	FfxFsr1Context m_fsr1Context;
	FfxFsr1QualityMode m_fsr1QualityMode{ FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY };
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
