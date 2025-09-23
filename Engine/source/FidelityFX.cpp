//Author: JEYOON YU
//Project: CubeEngine
//File: FidelityFX.cpp
#include "FidelityFX.hpp"

#include <stdexcept>

#include "DXHelper.hpp"
#include "DXRenderTarget.hpp"

#include "FidelityFX/host/backends/dx12/ffx_dx12.h"
//#include "FidelityFX-SDK-v1.1.4/ffx-api/include/ffx_api/dx12/ffx_api_dx12.hpp" 

// FidelityFX SDK 1.1.4
FidelityFX::~FidelityFX()
{
	ffxCasContextDestroy(&m_casContext);

	// Destroy FidelityFX interface memory
	free(m_casContextDesc.backendInterface.scratchBuffer);
}

void FidelityFX::CreateCasContext(
	const ComPtr<ID3D12Device>& device,
	int displayWidth, int displayHeight
)
{
	m_displayWidth = displayWidth;
	m_displayHeight = displayHeight;

	m_postProcessTexture.Reset();
	auto textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_displayWidth,
		m_displayHeight,
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_postProcessTexture)
	));
	m_postProcessTexture->SetName(L"CAS Post Process Texture");

	// https://gpuopen.com/manuals/fidelityfx_sdk/techniques/contrast-adaptive-sharpening/
	// Based on casrendermodule.cpp from FidelityFX SDK 1.1.4 CAS Sample
	if (m_casContextDesc.backendInterface.scratchBuffer != nullptr)
		free(m_casContextDesc.backendInterface.scratchBuffer);

	// Setup FidelityFX interface
	const size_t scratchBufferSize = ffxGetScratchMemorySizeDX12(FFX_CAS_CONTEXT_COUNT);
	void* scratchBuffer = calloc(scratchBufferSize, 1u);
	FfxErrorCode errorCode = ffxGetInterfaceDX12(&m_casContextDesc.backendInterface, device.Get(), scratchBuffer, scratchBufferSize, FFX_CAS_CONTEXT_COUNT);
	//CAULDRON_ASSERT(errorCode == FFX_OK);
	if (errorCode != FFX_OK) throw std::runtime_error("Failed to get FidelityFX interface.");
	// @TODO Create proper log
	//CauldronAssert(ASSERT_CRITICAL, m_casContextDesc.backendInterface.fpGetSDKVersion(&m_casContextDesc.backendInterface) == FFX_SDK_MAKE_VERSION(1, 1, 4),
	//	L"FidelityFX CAS 2.1 sample requires linking with a 1.1.4 version SDK backend");
	//CauldronAssert(ASSERT_CRITICAL, ffxCasGetEffectVersion() == FFX_SDK_MAKE_VERSION(1, 2, 0),
	//	L"FidelityFX CAS 2.1 sample requires linking with a 1.2 version FidelityFX CAS library");

	//m_casContextDesc.backendInterface.fpRegisterConstantBufferAllocator(&m_casContextDesc.backendInterface, SDKWrapper::ffxAllocateConstantBuffer);

	// Initialize CAS Context
	m_renderWidth = m_displayWidth;
	m_renderHeight = m_displayHeight;

	// Sharpening & Upscaling
	if (m_casEnableUpscaling)
	{
		m_renderWidth = static_cast<uint32_t>(static_cast<float>(m_displayWidth) / m_casUpscaleRatio);
		m_renderHeight = static_cast<uint32_t>(static_cast<float>(m_displayHeight) / m_casUpscaleRatio);
		m_casContextDesc.flags &= ~FFX_CAS_SHARPEN_ONLY;
	}
	// Sharpening Only
	else
		m_casContextDesc.flags |= FFX_CAS_SHARPEN_ONLY;

	// CubeEngine is using non-linear color space
	//m_casContextDesc.colorSpaceConversion = FFX_CAS_COLOR_SPACE_LINEAR;

	m_casContextDesc.maxRenderSize.width = m_renderWidth;
	m_casContextDesc.maxRenderSize.height = m_renderHeight;
	m_casContextDesc.displaySize.width = m_displayWidth;
	m_casContextDesc.displaySize.height = m_displayHeight;

	// Create CAS Context
	ffxCasContextCreate(&m_casContext, &m_casContextDesc);
	if (errorCode != FFX_OK) throw std::runtime_error("Failed to create CAS context");
}

void FidelityFX::CreateFSR1Context(
	const ComPtr<ID3D12Device>& device,
	int displayWidth, int displayHeight
)
{
	m_displayWidth = displayWidth;
	m_displayHeight = displayHeight;

	m_postProcessTexture.Reset();
	auto textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_displayWidth,
		m_displayHeight,
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_postProcessTexture)
	));
	m_postProcessTexture->SetName(L"FSR1 Post Process Texture");

	if (m_fsr1ContextDesc.backendInterface.scratchBuffer != nullptr)
		free(m_fsr1ContextDesc.backendInterface.scratchBuffer);

	// Setup FidelityFX interface
	const size_t scratchBufferSize = ffxGetScratchMemorySizeDX12(FFX_FSR1_CONTEXT_COUNT);
	void* scratchBuffer = calloc(scratchBufferSize, 1u);
	FfxErrorCode errorCode = ffxGetInterfaceDX12(&m_fsr1ContextDesc.backendInterface, device.Get(), scratchBuffer, scratchBufferSize, FFX_FSR1_CONTEXT_COUNT);
	//CAULDRON_ASSERT(errorCode == FFX_OK);
	if (errorCode != FFX_OK) throw std::runtime_error("Failed to get FidelityFX interface");
	// @TODO Create proper log
	//CauldronAssert(ASSERT_CRITICAL, m_fsr1ContextDesc.backendInterface.fpGetSDKVersion(&m_fsr1ContextDesc.backendInterface) == FFX_SDK_MAKE_VERSION(1, 1, 4),
	//	L"FidelityFX CAS 2.1 sample requires linking with a 1.1.4 version SDK backend");
	//CauldronAssert(ASSERT_CRITICAL, ffxCasGetEffectVersion() == FFX_SDK_MAKE_VERSION(1, 2, 0),
	//	L"FidelityFX CAS 2.1 sample requires linking with a 1.2 version FidelityFX CAS library");

	//m_fsr1ContextDesc.backendInterface.fpRegisterConstantBufferAllocator(&m_initializationParameters.backendInterface, SDKWrapper::ffxAllocateConstantBuffer);

	// @TODO ffxFsr1GetRenderResolutionFromQualityMode
	errorCode = ffxFsr1GetRenderResolutionFromQualityMode(&m_renderWidth, &m_renderHeight, m_displayWidth, m_displayHeight, m_fsr1QualityMode);
	if (errorCode != FFX_OK) throw std::runtime_error("Failed to get FSR1 render resolution");

	// Sharpening & Upscaling
	m_fsr1ContextDesc.flags = 0;
	if (m_enableRCAS)
	{
		m_fsr1ContextDesc.flags |= FFX_FSR1_ENABLE_RCAS;
	}

	m_fsr1ContextDesc.maxRenderSize.width = m_renderWidth;
	m_fsr1ContextDesc.maxRenderSize.height = m_renderHeight;
	m_fsr1ContextDesc.displaySize.width = m_displayWidth;
	m_fsr1ContextDesc.displaySize.height = m_displayHeight;
	m_fsr1ContextDesc.outputFormat = FFX_SURFACE_FORMAT_R8G8B8A8_UNORM;

	// @TODO Should I enable HDR?
	//m_fsr1ContextDesc.flags = FFX_FSR1_ENABLE_HIGH_DYNAMIC_RANGE;

	errorCode = ffxFsr1ContextCreate(&m_fsr1Context, &m_fsr1ContextDesc);
	if (errorCode != FFX_OK) throw std::runtime_error("Failed to create FSR1 context");
}

void FidelityFX::OnResize(
	const ComPtr<ID3D12Device>& device,
	int displayWidth, int displayHeight
)
{
	m_displayWidth = displayWidth;
	m_displayHeight = displayHeight;
	m_renderWidth = m_displayWidth;
	m_renderHeight = m_displayHeight;

	if (m_enableFFX)
	{
		if (m_enableFSR1)
		{
			ffxFsr1ContextDestroy(&m_fsr1Context);
			CreateFSR1Context(device, m_displayWidth, m_displayHeight);
		}
		else
		{
			ffxCasContextDestroy(&m_casContext);
			CreateCasContext(device, m_displayWidth, m_displayHeight);
		}
	}
}

void FidelityFX::UpdateScalePreset(const ComPtr<ID3D12Device>& device, bool enableFSR1, bool enableUpscaling, CASScalePreset preset)
{
	m_enableFSR1 = enableFSR1;
	m_casEnableUpscaling = enableUpscaling;
	m_casScalePreset = preset;

	switch (m_casScalePreset)
	{
	case CASScalePreset::UltraQuality: m_casUpscaleRatio = 1.3f; break;
	case CASScalePreset::Quality: m_casUpscaleRatio = 1.5f; break;
	case CASScalePreset::Balanced: m_casUpscaleRatio = 1.7f; break;
	case CASScalePreset::Performance: m_casUpscaleRatio = 2.f; break;
	case CASScalePreset::UltraPerformance: m_casUpscaleRatio = 3.f; break;
	case CASScalePreset::Custom: break;
	}
	OnResize(device, m_displayWidth, m_displayHeight);
}

void FidelityFX::UpdateScalePreset(const ComPtr<ID3D12Device>& device, bool enableFSR1, bool enableRCAS, FfxFsr1QualityMode preset)
{
	m_enableFSR1 = enableFSR1;
	m_enableRCAS = enableRCAS;
	m_fsr1QualityMode = preset;
	m_casUpscaleRatio = ffxFsr1GetUpscaleRatioFromQualityMode(m_fsr1QualityMode);
	OnResize(device, m_displayWidth, m_displayHeight);
}

void FidelityFX::Execute(
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12Resource>& inputRenderTarget,
	const ComPtr<ID3D12Resource>& outputRenderTarget
)
{
	// FSR1
	if (m_enableFSR1)
	{
		FfxFsr1DispatchDescription dispatchParameters{};
		dispatchParameters.commandList = ffxGetCommandListDX12(commandList.Get());
		dispatchParameters.renderSize = { m_fsr1ContextDesc.maxRenderSize.width, m_fsr1ContextDesc.maxRenderSize.height };
		dispatchParameters.enableSharpening = m_enableRCAS;
		dispatchParameters.sharpness = m_sharpness;

		CD3DX12_RESOURCE_BARRIER dispatchBarriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(inputRenderTarget.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		};
		commandList->ResourceBarrier(_countof(dispatchBarriers), dispatchBarriers);

		D3D12_RESOURCE_DESC inputResourceDesc = inputRenderTarget->GetDesc();
		FfxResourceDescription inputDesc = { FFX_RESOURCE_TYPE_TEXTURE2D, ffxGetSurfaceFormatDX12(inputResourceDesc.Format), static_cast<uint32_t>(inputResourceDesc.Width), static_cast<uint32_t>(inputResourceDesc.Height), 1, 0, FFX_RESOURCE_FLAGS_NONE, FFX_RESOURCE_USAGE_READ_ONLY };
		dispatchParameters.color = ffxGetResourceDX12(inputRenderTarget.Get(), inputDesc, L"FSR1_Input", FFX_RESOURCE_STATE_COMPUTE_READ);

		D3D12_RESOURCE_DESC outputResourceDesc = m_postProcessTexture->GetDesc();
		FfxResourceDescription outputDesc = { FFX_RESOURCE_TYPE_TEXTURE2D, ffxGetSurfaceFormatDX12(outputResourceDesc.Format), static_cast<uint32_t>(outputResourceDesc.Width), static_cast<uint32_t>(outputResourceDesc.Height), 1, 0, FFX_RESOURCE_FLAGS_NONE, FFX_RESOURCE_USAGE_UAV };
		dispatchParameters.output = ffxGetResourceDX12(m_postProcessTexture.Get(), outputDesc, L"FSR1_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);

		FfxErrorCode errorCode = ffxFsr1ContextDispatch(&m_fsr1Context, &dispatchParameters);
		if (errorCode != FFX_OK) throw std::runtime_error("Failed to dispatch FSR1");

		D3D12_RESOURCE_STATES outputBeforeState;
		if (inputRenderTarget.Get() != outputRenderTarget.Get()) outputBeforeState = D3D12_RESOURCE_STATE_PRESENT;
		else outputBeforeState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		CD3DX12_RESOURCE_BARRIER copyBarriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(outputRenderTarget.Get(), outputBeforeState, D3D12_RESOURCE_STATE_COPY_DEST)
		};
		commandList->ResourceBarrier(_countof(copyBarriers), copyBarriers);

		commandList->CopyResource(outputRenderTarget.Get(), m_postProcessTexture.Get());

		if (inputRenderTarget.Get() != outputRenderTarget.Get())
		{
			CD3DX12_RESOURCE_BARRIER finalBarriers[] = {
				CD3DX12_RESOURCE_BARRIER::Transition(inputRenderTarget.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON),
				CD3DX12_RESOURCE_BARRIER::Transition(outputRenderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
				CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON)
			};
			commandList->ResourceBarrier(_countof(finalBarriers), finalBarriers);
		}
		else
		{
			CD3DX12_RESOURCE_BARRIER finalBarriers[] = {
				CD3DX12_RESOURCE_BARRIER::Transition(outputRenderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
				CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON)
			};
			commandList->ResourceBarrier(_countof(finalBarriers), finalBarriers);
		}
	}
	// CAS
	else
	{
		FfxCasDispatchDescription dispatchParameters{};
		dispatchParameters.commandList = ffxGetCommandListDX12(commandList.Get());
		dispatchParameters.renderSize = { m_casContextDesc.maxRenderSize.width, m_casContextDesc.maxRenderSize.height };
		dispatchParameters.sharpness = m_sharpness;

		CD3DX12_RESOURCE_BARRIER dispatchBarriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(inputRenderTarget.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		};
		commandList->ResourceBarrier(_countof(dispatchBarriers), dispatchBarriers);

		D3D12_RESOURCE_DESC inputResourceDesc = inputRenderTarget->GetDesc();
		FfxResourceDescription inputDesc = { FFX_RESOURCE_TYPE_TEXTURE2D, ffxGetSurfaceFormatDX12(inputResourceDesc.Format), static_cast<uint32_t>(inputResourceDesc.Width), static_cast<uint32_t>(inputResourceDesc.Height), 1, 0, FFX_RESOURCE_FLAGS_NONE, FFX_RESOURCE_USAGE_READ_ONLY };
		dispatchParameters.color = ffxGetResourceDX12(inputRenderTarget.Get(), inputDesc, L"CAS_Input", FFX_RESOURCE_STATE_COMPUTE_READ);

		D3D12_RESOURCE_DESC outputResourceDesc = m_postProcessTexture->GetDesc();
		FfxResourceDescription outputDesc = { FFX_RESOURCE_TYPE_TEXTURE2D, ffxGetSurfaceFormatDX12(outputResourceDesc.Format), static_cast<uint32_t>(outputResourceDesc.Width), static_cast<uint32_t>(outputResourceDesc.Height), 1, 0, FFX_RESOURCE_FLAGS_NONE, FFX_RESOURCE_USAGE_UAV };
		dispatchParameters.output = ffxGetResourceDX12(m_postProcessTexture.Get(), outputDesc, L"CAS_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);

		FfxErrorCode errorCode = ffxCasContextDispatch(&m_casContext, &dispatchParameters);
		if (errorCode != FFX_OK) throw std::runtime_error("Failed to dispatch CAS");

		D3D12_RESOURCE_STATES outputBeforeState;
		if (inputRenderTarget.Get() != outputRenderTarget.Get()) outputBeforeState = D3D12_RESOURCE_STATE_PRESENT;
		else outputBeforeState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		CD3DX12_RESOURCE_BARRIER copyBarriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(outputRenderTarget.Get(), outputBeforeState, D3D12_RESOURCE_STATE_COPY_DEST)
		};
		commandList->ResourceBarrier(_countof(copyBarriers), copyBarriers);

		commandList->CopyResource(outputRenderTarget.Get(), m_postProcessTexture.Get());

		if (inputRenderTarget.Get() != outputRenderTarget.Get())
		{
			CD3DX12_RESOURCE_BARRIER finalBarriers[] = {
				CD3DX12_RESOURCE_BARRIER::Transition(inputRenderTarget.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON),
				CD3DX12_RESOURCE_BARRIER::Transition(outputRenderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
				CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON)
			};
			commandList->ResourceBarrier(_countof(finalBarriers), finalBarriers);
		}
		else
		{
			CD3DX12_RESOURCE_BARRIER finalBarriers[] = {
				CD3DX12_RESOURCE_BARRIER::Transition(outputRenderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
				CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON)
			};
			commandList->ResourceBarrier(_countof(finalBarriers), finalBarriers);
		}
	}
}

// @TODO FidelityFX SDK 2.0
//#include "Kits/FidelityFX/upscalers/include/ffx_upscale.hpp"

// @TODO FidelityFX SDK 2.0
// @TODO Link amd_fidelityfx_loader_dx12.lib!!!
//void FidelityFX::CreateContext(const ComPtr<ID3D12Device>& device)
//{
//	// https://gpuopen.com/manuals/fidelityfx_sdk2/getting-started/ffx-api/
//	// Based on fsrapirendermodule.cpp from FidelityFX SDK 2.0 FSR Sample
//	// Backend Creation
//	ffx::CreateBackendDX12Desc backendDesc{};
//	backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
//	backendDesc.device = device.Get();
//
//	ffx::CreateContextDescUpscale createFsr{};
//	ffx::ReturnCode retCode;
//	ffxReturnCode_t retCode_t;
//
//	// @TODO No magic numbers!
//	createFsr.maxRenderSize = { 1280, 720 };
//	createFsr.maxUpscaleSize = { 2560, 1440 };
//	/*
//	* FFX_UPSCALE_ENABLE_DYNAMIC_RESOLUTION
//	* FFX_UPSCALE_ENABLE_DEPTH_INVERTED
//	* FFX_UPSCALE_ENABLE_DEPTH_INFINITE
//	* These flags can be considered if the engine supports
//	* Dynamic Resolution or Inverted Depth
//	*/
//	createFsr.flags = FFX_UPSCALE_ENABLE_AUTO_EXPOSURE;
//	createFsr.flags |= FFX_UPSCALE_ENABLE_DEBUG_VISUALIZATION;
//	createFsr.flags |= FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE;
//	createFsr.fpMessage = nullptr;
//
//	// Create the FSR Context
//	{
//		// Query VRAM size
//		FfxApiEffectMemoryUsage gpuMemoryUsageUpscaler{ 0 };
//		ffxQueryDescUpscaleGetGPUMemoryUsageV2 upscalerGetGPUMemoryUsageV2{ 0 };
//		upscalerGetGPUMemoryUsageV2.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GPU_MEMORY_USAGE_V2;
//		upscalerGetGPUMemoryUsageV2.device = device.Get();
//
//		// @TODO No magic numbers!
//		upscalerGetGPUMemoryUsageV2.maxRenderSize = { 1280, 720 };
//		upscalerGetGPUMemoryUsageV2.maxUpscaleSize = { 2560, 1440 };
//		upscalerGetGPUMemoryUsageV2.flags = createFsr.flags;
//		upscalerGetGPUMemoryUsageV2.gpuMemoryUsageUpscaler = &gpuMemoryUsageUpscaler;
//
//		ffxOverrideVersion versionOverride{ 0 };
//		versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
//		if (m_fsrVersionIndex < m_fsrVersionIds.size() && m_overrideVersion)
//		{
//			versionOverride.versionId = m_fsrVersionIds[m_fsrVersionIndex];
//			upscalerGetGPUMemoryUsageV2.header.pNext = &versionOverride.header;
//			retCode_t = ffxQuery(nullptr, &upscalerGetGPUMemoryUsageV2.header);
//			// @TODO Create proper log
//			//CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
//			//	L"ffxQuery(nullptr,UpscaleGetGPUMemoryUsageV2, %S) returned %d", m_fsrVersionNames[m_fsrVersionIndex], retCode_t);
//			//CAUDRON_LOG_INFO(L"Upscaler version %S Query GPUMemoryUsageV2 VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", m_fsrVersionNames[m_fsrVersionIndex], gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);
//			retCode = ffx::CreateContext(m_upscalingContext, nullptr, createFsr, backendDesc, versionOverride);
//		}
//		else
//		{
//			retCode = ffx::Query(upscalerGetGPUMemoryUsageV2);
//			// @TODO Create proper log
//			//CauldronAssert(ASSERT_WARNING, retCode == ffx::ReturnCode::Ok,
//			//	L"ffxQuery(nullptr,UpscaleGetGPUMemoryUsageV2) returned %d", (uint32_t)retCode);
//			//CAUDRON_LOG_INFO(L"Default Upscaler Query GPUMemoryUsageV2 totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);
//			retCode = ffx::CreateContext(m_upscalingContext, nullptr, createFsr, backendDesc);
//		}
//
//		if (retCode_t != FFX_API_RETURN_OK) throw std::runtime_error("Couldn't create the ffxapi upscaling context:" + std::to_string(retCode_t));
//	}
//
//	// Query Created Version
//	ffxQueryGetProviderVersion getVersion{ 0 };
//	getVersion.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
//
//	retCode_t = ffxQuery(&m_upscalingContext, &getVersion.header);
//	// @TODO Create proper log
//	//CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
//	//	L"ffxQuery(UpscalingContext,GetProviderVersion) returned %d", retCode_t);
//
//	m_currentUpscaleContextVersionId = getVersion.versionId;
//	m_currentUpscaleContextVersionName = getVersion.versionName;
//
//	// @TODO Create proper log
//	//CAUDRON_LOG_INFO(L"Upscaler Context versionid 0x%016llx, %S", m_currentUpscaleContextVersionId, m_currentUpscaleContextVersionName);
//
//	for (uint32_t i = 0; i < m_fsrVersionIds.size(); i++)
//	{
//		if (m_fsrVersionIds[i] == m_currentUpscaleContextVersionId)
//		{
//			m_fsrVersionIndex = i;
//		}
//	}
//}
//
//void FidelityFX::Execute(
//	const ComPtr<ID3D12GraphicsCommandList>& commandList,
//	const ComPtr<ID3D12DescriptorHeap>& srvHeap,
//	const std::unique_ptr<DXRenderTarget>& dxRenderTarget,
//	const ComPtr<ID3D12Resource>& renderTarget
//)
//{
//	CD3DX12_RESOURCE_BARRIER barriers[2];
//	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(dxRenderTarget->GetMSAARenderTarget().Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
//	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandList->ResourceBarrier(2, barriers);
//
//	//{
//	//	// FFXAPI
//	//	// All cauldron resources come into a render module in a generic read state (ResourceState::NonPixelShaderResource | ResourceState::PixelShaderResource)
//	//	ffx::DispatchDescUpscale dispatchUpscale{};
//	//	dispatchUpscale.commandList = commandList.Get();
//	//	dispatchUpscale.color = SDKWrapper::ffxGetResourceApi(m_pTempTexture->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
//	//	//dispatchUpscale.depth = SDKWrapper::ffxGetResourceApi(m_pDepthTarget->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
//	//	//dispatchUpscale.motionVectors = SDKWrapper::ffxGetResourceApi(m_pMotionVectors->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
//	//	dispatchUpscale.exposure = SDKWrapper::ffxGetResourceApi(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
//	//	dispatchUpscale.output = SDKWrapper::ffxGetResourceApi(m_pColorTarget->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
//
//	//	if (m_MaskMode != FSRMaskMode::Disabled)
//	//	{
//	//		dispatchUpscale.reactive = SDKWrapper::ffxGetResourceApi(m_pReactiveMask->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
//	//	}
//	//	else
//	//	{
//	//		dispatchUpscale.reactive = SDKWrapper::ffxGetResourceApi(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
//	//	}
//
//	//	if (m_UseMask)
//	//	{
//	//		dispatchUpscale.transparencyAndComposition =
//	//			SDKWrapper::ffxGetResourceApi(m_pCompositionMask->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
//	//	}
//	//	else
//	//	{
//	//		dispatchUpscale.transparencyAndComposition = SDKWrapper::ffxGetResourceApi(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
//	//	}
//
//	//	// Jitter is calculated earlier in the frame using a callback from the camera update
//	//	dispatchUpscale.jitterOffset.x = -m_JitterX;
//	//	dispatchUpscale.jitterOffset.y = -m_JitterY;
//	//	dispatchUpscale.motionVectorScale.x = resInfo.fRenderWidth();
//	//	dispatchUpscale.motionVectorScale.y = resInfo.fRenderHeight();
//	//	dispatchUpscale.reset = m_ResetUpscale || GetScene()->GetCurrentCamera()->WasCameraReset();
//	//	dispatchUpscale.enableSharpening = m_RCASSharpen;
//	//	dispatchUpscale.sharpness = m_Sharpness;
//
//	//	// Cauldron keeps time in seconds, but FSR expects milliseconds
//	//	dispatchUpscale.frameTimeDelta = static_cast<float>(deltaTime * 1000.f);
//
//	//	dispatchUpscale.preExposure = GetScene()->GetSceneExposure();
//	//	dispatchUpscale.renderSize.width = resInfo.RenderWidth;
//	//	dispatchUpscale.renderSize.height = resInfo.RenderHeight;
//	//	dispatchUpscale.upscaleSize.width = resInfo.UpscaleWidth;
//	//	dispatchUpscale.upscaleSize.height = resInfo.UpscaleHeight;
//
//	//	// Setup camera params as required
//	//	dispatchUpscale.cameraFovAngleVertical = pCamera->GetFovY();
//
//	//	if (s_InvertedDepth)
//	//	{
//	//		dispatchUpscale.cameraFar = pCamera->GetNearPlane();
//	//		dispatchUpscale.cameraNear = FLT_MAX;
//	//	}
//	//	else
//	//	{
//	//		dispatchUpscale.cameraFar = pCamera->GetFarPlane();
//	//		dispatchUpscale.cameraNear = pCamera->GetNearPlane();
//	//	}
//	//	dispatchUpscale.flags = 0;
//	//	dispatchUpscale.flags |= m_DrawUpscalerDebugView ? FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW : 0;
//	//	dispatchUpscale.flags |= m_colorSpace == FSRColorSpace::sRGBColorSpace ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_SRGB : 0;
//	//	dispatchUpscale.flags |= m_colorSpace == FSRColorSpace::PQColorSpace ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_PQ : 0;
//
//	//	ffx::ReturnCode retCode = ffx::Dispatch(m_UpscalingContext, dispatchUpscale);
//	//	CauldronAssert(ASSERT_CRITICAL, !!retCode, L"Dispatching FSR upscaling failed: %d", (uint32_t)retCode);
//	//}
//}
//
//void FidelityFX::QueryVersion(const ComPtr<ID3D12Device>& device)
//{
//	// Get version info from ffxapi
//	ffxQueryDescGetVersions versionQuery{ 0 };
//	versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
//
//	versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
//	versionQuery.device = device.Get();
//
//	uint64_t versionCount{ 0 };
//	versionQuery.outputCount = &versionCount;
//	ffxReturnCode_t retCode_t = ffxQuery(nullptr, &versionQuery.header);
//	// @TODO Create proper log
//	//CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
//	//	L"ffxQuery(nullptr,GetVersions) returned %d", retCode_t);
//
//	m_fsrVersionIds.resize(versionCount);
//	m_fsrVersionNames.resize(versionCount);
//	versionQuery.versionIds = m_fsrVersionIds.data();
//	versionQuery.versionNames = m_fsrVersionNames.data();
//	retCode_t = ffxQuery(nullptr, &versionQuery.header);
//	// @TODO Create proper log
//	//CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
//	//	L"ffxQuery(nullptr,GetVersions) returned %d", retCode_t);
//}
