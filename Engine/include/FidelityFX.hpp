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

	// CAS
	void CreateCasContext(
		const ComPtr<ID3D12Device>& device,
		int displayWidth, int displayHeight
	);
	/*
	 * FSR1 -> enableFSR1 == true
	 * CAS -> enableFSR1 == false
	 */
	void UpdateScalePreset(
		const ComPtr<ID3D12Device>& device,
		bool enableFSR1,
		bool enableUpscaling,
		CASScalePreset preset);

	bool GetEnableUpscaling() const { return m_casEnableUpscaling; }
	CASScalePreset GetScalePreset() const { return m_casScalePreset; }

	// FSR1
	void CreateFSR1Context(
		const ComPtr<ID3D12Device>& device,
		int displayWidth, int displayHeight
	);
	void UpdateScalePreset(
		const ComPtr<ID3D12Device>& device,
		bool enableFSR1,
		bool enableRCAS,
		FfxFsr1QualityMode preset);

	bool GetEnableFSR1() const { return m_enableFSR1; }
	bool GetEnableRCAS() const { return m_enableRCAS; }
	FfxFsr1QualityMode GetFSR1QualityMode() const { return m_fsr1QualityMode; }

	// Common
	void SetEnableFFX(bool enableFFX) { m_enableFFX = enableFFX; }
	void OnResize(
		const ComPtr<ID3D12Device>& device,
		int displayWidth, int displayHeight
	);
	void Execute(
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12Resource>& inputRenderTarget,
		const ComPtr<ID3D12Resource>& outputRenderTarget
	);

	bool GetEnableFFX() const { return m_enableFFX; };
	uint32_t GetRenderWidth() const { return m_renderWidth; }
	uint32_t GetRenderHeight() const { return m_renderHeight; }
private:
	// FidelityFX SDK 1.1.4
	// CAS
	FfxCasContextDescription m_casContextDesc{};
	FfxCasContext m_casContext;

	bool m_casEnableUpscaling{ false };
	CASScalePreset m_casScalePreset{ CASScalePreset::UltraQuality };
	float m_casUpscaleRatio{ 1.3f };

	// FSR1
	// @TODO FfxFsr1QualityMode
	FfxFsr1ContextDescription m_fsr1ContextDesc{};
	FfxFsr1Context m_fsr1Context;
	bool m_enableRCAS{ true };
	FfxFsr1QualityMode m_fsr1QualityMode{ FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY };

	// Common
	bool m_enableFFX{ false };
	ComPtr<ID3D12Resource> m_postProcessTexture;
	bool m_enableFSR1{ false };
	float m_sharpness{ 0.2f };
	int m_displayWidth, m_displayHeight;
	uint32_t m_renderWidth, m_renderHeight;
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
