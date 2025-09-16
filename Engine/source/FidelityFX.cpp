//Author: JEYOON YU
//Project: CubeEngine
//File: FidelityFX.cpp
#include "FidelityFX.hpp"

#include <stdexcept>

#include "DXRenderTarget.hpp"

#include "Kits/FidelityFX/upscalers/include/ffx_upscale.hpp"

// @TODO Link amd_fidelityfx_loader_dx12.lib!!!
void FidelityFX::CreateContext(const ComPtr<ID3D12Device>& device)
{
	// https://gpuopen.com/manuals/fidelityfx_sdk2/getting-started/ffx-api/
	// Based on fsrapirendermodule.cpp from FidelityFX SDK 2.0 FSR Sample
	// Backend Creation
	ffx::CreateBackendDX12Desc backendDesc{};
	backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
	backendDesc.device = device.Get();

	ffx::CreateContextDescUpscale createFsr{};
	ffx::ReturnCode retCode;
	ffxReturnCode_t retCode_t;

	// @TODO No magic numbers!
	createFsr.maxRenderSize = { 1280, 720 };
	createFsr.maxUpscaleSize = { 2560, 1440 };
	/*
	* FFX_UPSCALE_ENABLE_DYNAMIC_RESOLUTION
	* FFX_UPSCALE_ENABLE_DEPTH_INVERTED
	* FFX_UPSCALE_ENABLE_DEPTH_INFINITE
	* These flags can be considered if the engine supports
	* Dynamic Resolution or Inverted Depth
	*/
	createFsr.flags = FFX_UPSCALE_ENABLE_AUTO_EXPOSURE;
	createFsr.flags |= FFX_UPSCALE_ENABLE_DEBUG_VISUALIZATION;
	createFsr.flags |= FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE;
	createFsr.fpMessage = nullptr;

	// Create the FSR Context
	{
		// Query VRAM size
		FfxApiEffectMemoryUsage gpuMemoryUsageUpscaler{ 0 };
		ffxQueryDescUpscaleGetGPUMemoryUsageV2 upscalerGetGPUMemoryUsageV2{ 0 };
		upscalerGetGPUMemoryUsageV2.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GPU_MEMORY_USAGE_V2;
		upscalerGetGPUMemoryUsageV2.device = device.Get();

		// @TODO No magic numbers!
		upscalerGetGPUMemoryUsageV2.maxRenderSize = { 1280, 720 };
		upscalerGetGPUMemoryUsageV2.maxUpscaleSize = { 2560, 1440 };
		upscalerGetGPUMemoryUsageV2.flags = createFsr.flags;
		upscalerGetGPUMemoryUsageV2.gpuMemoryUsageUpscaler = &gpuMemoryUsageUpscaler;

		ffxOverrideVersion versionOverride{ 0 };
		versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
		if (m_fsrVersionIndex < m_fsrVersionIds.size() && m_overrideVersion)
		{
			versionOverride.versionId = m_fsrVersionIds[m_fsrVersionIndex];
			upscalerGetGPUMemoryUsageV2.header.pNext = &versionOverride.header;
			retCode_t = ffxQuery(nullptr, &upscalerGetGPUMemoryUsageV2.header);
			// @TODO Create proper log
			//CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
			//	L"ffxQuery(nullptr,UpscaleGetGPUMemoryUsageV2, %S) returned %d", m_fsrVersionNames[m_fsrVersionIndex], retCode_t);
			//CAUDRON_LOG_INFO(L"Upscaler version %S Query GPUMemoryUsageV2 VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", m_fsrVersionNames[m_fsrVersionIndex], gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);
			retCode = ffx::CreateContext(m_upscalingContext, nullptr, createFsr, backendDesc, versionOverride);
		}
		else
		{
			retCode = ffx::Query(upscalerGetGPUMemoryUsageV2);
			// @TODO Create proper log
			//CauldronAssert(ASSERT_WARNING, retCode == ffx::ReturnCode::Ok,
			//	L"ffxQuery(nullptr,UpscaleGetGPUMemoryUsageV2) returned %d", (uint32_t)retCode);
			//CAUDRON_LOG_INFO(L"Default Upscaler Query GPUMemoryUsageV2 totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);
			retCode = ffx::CreateContext(m_upscalingContext, nullptr, createFsr, backendDesc);
		}

		if (retCode_t != FFX_API_RETURN_OK) throw std::runtime_error("Couldn't create the ffxapi upscaling context:" + std::to_string(retCode_t));
	}

	// Query Created Version
	ffxQueryGetProviderVersion getVersion{ 0 };
	getVersion.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;

	retCode_t = ffxQuery(&m_upscalingContext, &getVersion.header);
	// @TODO Create proper log
	//CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
	//	L"ffxQuery(UpscalingContext,GetProviderVersion) returned %d", retCode_t);

	m_currentUpscaleContextVersionId = getVersion.versionId;
	m_currentUpscaleContextVersionName = getVersion.versionName;

	// @TODO Create proper log
	//CAUDRON_LOG_INFO(L"Upscaler Context versionid 0x%016llx, %S", m_currentUpscaleContextVersionId, m_currentUpscaleContextVersionName);

	for (uint32_t i = 0; i < m_fsrVersionIds.size(); i++)
	{
		if (m_fsrVersionIds[i] == m_currentUpscaleContextVersionId)
		{
			m_fsrVersionIndex = i;
		}
	}
}

void FidelityFX::Execute(
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12DescriptorHeap>& srvHeap,
	const std::unique_ptr<DXRenderTarget>& dxRenderTarget,
	const ComPtr<ID3D12Resource>& renderTarget
)
{
	CD3DX12_RESOURCE_BARRIER barriers[2];
	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(dxRenderTarget->GetMSAARenderTarget().Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ResourceBarrier(2, barriers);

	//{
	//	// FFXAPI
	//	// All cauldron resources come into a render module in a generic read state (ResourceState::NonPixelShaderResource | ResourceState::PixelShaderResource)
	//	ffx::DispatchDescUpscale dispatchUpscale{};
	//	dispatchUpscale.commandList = commandList.Get();
	//	dispatchUpscale.color = SDKWrapper::ffxGetResourceApi(m_pTempTexture->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	//	//dispatchUpscale.depth = SDKWrapper::ffxGetResourceApi(m_pDepthTarget->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	//	//dispatchUpscale.motionVectors = SDKWrapper::ffxGetResourceApi(m_pMotionVectors->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	//	dispatchUpscale.exposure = SDKWrapper::ffxGetResourceApi(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	//	dispatchUpscale.output = SDKWrapper::ffxGetResourceApi(m_pColorTarget->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);

	//	if (m_MaskMode != FSRMaskMode::Disabled)
	//	{
	//		dispatchUpscale.reactive = SDKWrapper::ffxGetResourceApi(m_pReactiveMask->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	//	}
	//	else
	//	{
	//		dispatchUpscale.reactive = SDKWrapper::ffxGetResourceApi(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	//	}

	//	if (m_UseMask)
	//	{
	//		dispatchUpscale.transparencyAndComposition =
	//			SDKWrapper::ffxGetResourceApi(m_pCompositionMask->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	//	}
	//	else
	//	{
	//		dispatchUpscale.transparencyAndComposition = SDKWrapper::ffxGetResourceApi(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	//	}

	//	// Jitter is calculated earlier in the frame using a callback from the camera update
	//	dispatchUpscale.jitterOffset.x = -m_JitterX;
	//	dispatchUpscale.jitterOffset.y = -m_JitterY;
	//	dispatchUpscale.motionVectorScale.x = resInfo.fRenderWidth();
	//	dispatchUpscale.motionVectorScale.y = resInfo.fRenderHeight();
	//	dispatchUpscale.reset = m_ResetUpscale || GetScene()->GetCurrentCamera()->WasCameraReset();
	//	dispatchUpscale.enableSharpening = m_RCASSharpen;
	//	dispatchUpscale.sharpness = m_Sharpness;

	//	// Cauldron keeps time in seconds, but FSR expects milliseconds
	//	dispatchUpscale.frameTimeDelta = static_cast<float>(deltaTime * 1000.f);

	//	dispatchUpscale.preExposure = GetScene()->GetSceneExposure();
	//	dispatchUpscale.renderSize.width = resInfo.RenderWidth;
	//	dispatchUpscale.renderSize.height = resInfo.RenderHeight;
	//	dispatchUpscale.upscaleSize.width = resInfo.UpscaleWidth;
	//	dispatchUpscale.upscaleSize.height = resInfo.UpscaleHeight;

	//	// Setup camera params as required
	//	dispatchUpscale.cameraFovAngleVertical = pCamera->GetFovY();

	//	if (s_InvertedDepth)
	//	{
	//		dispatchUpscale.cameraFar = pCamera->GetNearPlane();
	//		dispatchUpscale.cameraNear = FLT_MAX;
	//	}
	//	else
	//	{
	//		dispatchUpscale.cameraFar = pCamera->GetFarPlane();
	//		dispatchUpscale.cameraNear = pCamera->GetNearPlane();
	//	}
	//	dispatchUpscale.flags = 0;
	//	dispatchUpscale.flags |= m_DrawUpscalerDebugView ? FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW : 0;
	//	dispatchUpscale.flags |= m_colorSpace == FSRColorSpace::sRGBColorSpace ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_SRGB : 0;
	//	dispatchUpscale.flags |= m_colorSpace == FSRColorSpace::PQColorSpace ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_PQ : 0;

	//	ffx::ReturnCode retCode = ffx::Dispatch(m_UpscalingContext, dispatchUpscale);
	//	CauldronAssert(ASSERT_CRITICAL, !!retCode, L"Dispatching FSR upscaling failed: %d", (uint32_t)retCode);
	//}
}

void FidelityFX::QueryVersion(const ComPtr<ID3D12Device>& device)
{
	// Get version info from ffxapi
	ffxQueryDescGetVersions versionQuery{ 0 };
	versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;

	versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
	versionQuery.device = device.Get();

	uint64_t versionCount{ 0 };
	versionQuery.outputCount = &versionCount;
	ffxReturnCode_t retCode_t = ffxQuery(nullptr, &versionQuery.header);
	// @TODO Create proper log
	//CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
	//	L"ffxQuery(nullptr,GetVersions) returned %d", retCode_t);

	m_fsrVersionIds.resize(versionCount);
	m_fsrVersionNames.resize(versionCount);
	versionQuery.versionIds = m_fsrVersionIds.data();
	versionQuery.versionNames = m_fsrVersionNames.data();
	retCode_t = ffxQuery(nullptr, &versionQuery.header);
	// @TODO Create proper log
	//CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
	//	L"ffxQuery(nullptr,GetVersions) returned %d", retCode_t);
}

