//Author: JEYOON YU
//Project: CubeEngine
//File: DXPostProcessContext.cpp
#include "DXPostProcessContext.hpp"
#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
#include "DXSkyboxRenderContext.hpp"
#include "Engine.hpp"

void DXPostProcessContext::Initialize()
{
	// Initialize FidelityFX
	m_fidelityFX = std::make_unique<FidelityFX>();
	m_fidelityFX->InitializeBackend(m_renderManager->m_device, m_renderManager->m_width, m_renderManager->m_height);
	m_fidelityFX->CreateCasContext();
	m_fidelityFX->CreateFSR1Context();

	// Tone Mapping Pipeline
	CD3DX12_DESCRIPTOR_RANGE1 srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[1].InitAsConstants(sizeof(PushConstants) / 4, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	// @TODO Use DXRenderManager's CreateRootSignature later
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 1;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(2, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	if (FAILED(hr))
	{
		if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
		Engine::GetLogger().LogError(LogCategory::D3D12, "Failed to serialize root signature.");
	}

	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	DXHelper::ThrowIfFailed(m_rootSignature->SetName(L"Tone Mapping Root Signature"));

	// RTV Format should be DXGI_FORMAT_R8G8B8A8_UNORM to render tone-mapped LDR output
	std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };
	m_pipeline = DXPipeLineBuilder(m_renderManager->m_device, m_rootSignature)
		.SetShaders("../Engine/shaders/hlsl/ToneMapping.vert.hlsl", "../Engine/shaders/hlsl/ToneMapping.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{})
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false)
		.SetDepthStencil(false, false)
		.SetRenderTargets(rtvFormats)
		.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.SetBlendMode(CD3DX12_BLEND_DESC(D3D12_DEFAULT).RenderTarget[0])
		.Build();
}

void DXPostProcessContext::OnResize()
{
	m_fidelityFX->OnResize(m_renderManager->m_device, m_renderManager->m_width, m_renderManager->m_height);
}

void DXPostProcessContext::Execute(ICommandListWrapper* commandListWrapper)
{
	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	auto* backBuffer = m_renderManager->m_renderTargets[m_renderManager->m_frameIndex].Get();
	ComPtr<ID3D12Resource> ldrRenderTarget = m_renderManager->m_renderTarget->GetLDRRenderTarget();
	ComPtr<ID3D12Resource> hdrRenderTarget = m_renderManager->m_renderTarget->GetHDRRenderTarget();

	// Forward Rendering (Resolve Only) HDR -> HDR Resolve
	if (!m_renderManager->m_deferredRenderingEnabled)
	{
		auto* msaaTarget = m_renderManager->m_renderTarget->GetMSAARenderTarget().Get();
		D3D12_RESOURCE_BARRIER barriers[2];
		// MSAA Target: RENDER_TARGET -> RESOLVE_SOURCE
		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(msaaTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
		// HDR Render Target: COMMON -> RESOLVE_DEST
		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(hdrRenderTarget.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RESOLVE_DEST);
		commandList->ResourceBarrier(2, barriers);

		// Resolve
		commandList->ResolveSubresource(hdrRenderTarget.Get(), 0, msaaTarget, 0, DXGI_FORMAT_R16G16B16A16_FLOAT);

		// HDR Render Target: RESOLVE_DEST -> COMMON
		auto barrierHDR = CD3DX12_RESOURCE_BARRIER::Transition(hdrRenderTarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COMMON);
		commandList->ResourceBarrier(1, &barrierHDR);
	}

	// Tone Mapping & FSR
	if (m_fidelityFX->GetCurrentEffect() != FidelityFX::UpscaleEffect::NONE)
	{
		// LDR Render Target : COMMON -> RENDER_TARGET
		auto barrierLDR = CD3DX12_RESOURCE_BARRIER::Transition(ldrRenderTarget.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrierLDR);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_renderManager->m_renderTarget->GetLDRRtvHeap()->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<float>(m_fidelityFX->GetRenderWidth()), static_cast<float>(m_fidelityFX->GetRenderHeight()), 0.f, 1.f };
		D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(m_fidelityFX->GetRenderWidth()), static_cast<LONG>(m_fidelityFX->GetRenderHeight()) };
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		commandList->SetPipelineState(m_pipeline->GetPipelineState().Get());
		commandList->SetGraphicsRootSignature(m_rootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { m_renderManager->m_srvHeap.Get() };
		commandList->SetDescriptorHeaps(1, ppHeaps);

		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvHandle.ptr += static_cast<UINT64>(m_renderManager->m_hdrSrvHandle.second) * m_renderManager->m_srvDescriptorSize;
		commandList->SetGraphicsRootDescriptorTable(0, srvHandle);

		pushConstants = {
			.exposure = m_exposure
		};
		commandList->SetGraphicsRoot32BitConstants(1, sizeof(PushConstants) / 4, &pushConstants, 0);

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->DrawInstanced(3, 1, 0, 0);

		// LDR Render Target : RENDER_TARGET -> NON_PIXEL_SHADER_RESOURCE
		barrierLDR = CD3DX12_RESOURCE_BARRIER::Transition(ldrRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &barrierLDR);

		m_fidelityFX->Execute(commandList, ldrRenderTarget.Get(), backBuffer);

		// LDR Render Target : NON_PIXEL_SHADER_RESOURCE -> COMMON
		barrierLDR = CD3DX12_RESOURCE_BARRIER::Transition(ldrRenderTarget.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
		commandList->ResourceBarrier(1, &barrierLDR);
	}
	// Tone Mapping
	else
	{
		// PRESENT == COMMON
		// Main Render Target: PRESENT -> RENDER_TARGET
		D3D12_RESOURCE_BARRIER barriers[2];
		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		// HDR Render Target: COMMON -> PIXEL_SHADER_RESOURCE (SRV)
		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(hdrRenderTarget.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(2, barriers);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderManager->m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(m_renderManager->m_frameIndex), m_renderManager->m_rtvDescriptorSize);
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<float>(m_renderManager->m_width), static_cast<float>(m_renderManager->m_height), 0.f, 1.f };
		D3D12_RECT scissorRect = { 0, 0, m_renderManager->m_width, m_renderManager->m_height };
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		commandList->SetPipelineState(m_pipeline->GetPipelineState().Get());
		commandList->SetGraphicsRootSignature(m_rootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { m_renderManager->m_srvHeap.Get() };
		commandList->SetDescriptorHeaps(1, ppHeaps);

		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvHandle.ptr += static_cast<UINT64>(m_renderManager->m_hdrSrvHandle.second) * m_renderManager->m_srvDescriptorSize;
		commandList->SetGraphicsRootDescriptorTable(0, srvHandle);

		pushConstants = {
			.exposure = m_exposure
		};
		commandList->SetGraphicsRoot32BitConstants(1, sizeof(PushConstants) / 4, &pushConstants, 0);

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->DrawInstanced(3, 1, 0, 0);

		// HDR Render Target: PIXEL_SHADER_RESOURCE (SRV) -> COMMON
		auto barrierHDR = CD3DX12_RESOURCE_BARRIER::Transition(hdrRenderTarget.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
		commandList->ResourceBarrier(1, &barrierHDR);
	}
}

void DXPostProcessContext::CleanUp()
{
	m_fidelityFX.reset();
}

void DXPostProcessContext::UpdateScalePreset(const FidelityFX::UpscaleEffect& effect, const FfxFsr1QualityMode& mode, const FidelityFX::CASScalePreset& preset) const
{
	m_renderManager->QueueDeferredFunction(
		[this, effect, mode, preset]() -> bool
		{
			m_renderManager->WaitForGPU();

			if (m_fidelityFX->UpdatePreset(effect, mode, preset))
			{
				m_fidelityFX->OnResize(m_renderManager->m_device, m_renderManager->m_width, m_renderManager->m_height);

				// @TODO Recreate Render Target should not be here! It's just temporary solution, must be handled properly in RenderManager
				m_renderManager->m_renderTarget.reset();
				m_renderManager->m_renderTarget = std::make_unique<DXRenderTarget>(
					m_renderManager->m_device, Engine::GetWindow().GetWindow(),
					m_fidelityFX->GetRenderWidth(),
					m_fidelityFX->GetRenderHeight(),
					m_renderManager->m_deferredRenderingEnabled
				);
				// Allocate SRV handle for tone mapping
				// @TODO This should be inside DXRenderTarget
				m_renderManager->m_renderTarget->CreateSRV(m_renderManager->m_hdrSrvHandle.first);

				if (m_renderManager->m_deferredRenderingEnabled)
				{
					m_renderManager->m_gBufferContext->OnResize();
					m_renderManager->m_globalLightingContext->OnResize();
					m_renderManager->m_localLightingContext->OnResize();
					m_renderManager->m_naiveLightingContext->OnResize();
				}
			}

			return true;
		}
	);
}

void DXPostProcessContext::DrawImGui()
{
	if (m_fidelityFX)
	{
		// FidelityFX
		auto currentEffect = m_fidelityFX->GetCurrentEffect();
		FfxFsr1QualityMode currentFsrMode = m_fidelityFX->GetFSR1QualityMode();
		FidelityFX::CASScalePreset currentCasScalePreset = m_fidelityFX->GetSCASScalePreset();
		static FidelityFX::UpscaleEffect lastActiveEffect = FidelityFX::UpscaleEffect::FSR1;
		if (currentEffect != FidelityFX::UpscaleEffect::NONE) lastActiveEffect = currentEffect;

		// Enable/Disable FidelityFX
		bool ffxEnabled = (currentEffect != FidelityFX::UpscaleEffect::NONE);
		int effectMode = (lastActiveEffect == FidelityFX::UpscaleEffect::FSR1) ? 1 : 2;
		if (ImGui::Checkbox("Enable FidelityFX", &ffxEnabled))
		{
			FidelityFX::UpscaleEffect newEffect = ffxEnabled ? lastActiveEffect : FidelityFX::UpscaleEffect::NONE;
			UpdateScalePreset(newEffect, currentFsrMode, currentCasScalePreset);
		}

		if (ffxEnabled)
		{
			// Enable FSR1/CAS
			if (ImGui::RadioButton("FidelityFX FSR1", &effectMode, 1))
			{
				lastActiveEffect = FidelityFX::UpscaleEffect::FSR1;
				UpdateScalePreset(FidelityFX::UpscaleEffect::FSR1, currentFsrMode, currentCasScalePreset);
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("FidelityFX CAS", &effectMode, 2))
			{
				lastActiveEffect = FidelityFX::UpscaleEffect::CAS_SHARPEN_ONLY;
				UpdateScalePreset(FidelityFX::UpscaleEffect::CAS_SHARPEN_ONLY, currentFsrMode, currentCasScalePreset);
			}

			bool rcasEnabled = m_fidelityFX->GetEnableRCAS();
			bool casUpscalingEnabled = currentEffect == FidelityFX::UpscaleEffect::CAS_UPSCALING;
			// FSR1
			if (currentEffect == FidelityFX::UpscaleEffect::FSR1)
			{
				if (ImGui::Checkbox("Enable FidelityFX RCAS", &rcasEnabled))
				{
					m_fidelityFX->SetEnableRCAS(rcasEnabled);
				}

				const char* fsrLabels[] = { "Ultra Quality", "Quality", "Balanced", "Performance" };
				int currentFsrModeInt = static_cast<int>(currentFsrMode);
				if (ImGui::Combo("Upscale Preset", &currentFsrModeInt, fsrLabels, IM_ARRAYSIZE(fsrLabels)))
				{
					UpdateScalePreset(FidelityFX::UpscaleEffect::FSR1, static_cast<FfxFsr1QualityMode>(currentFsrModeInt), currentCasScalePreset);
				}
			}
			// CAS
			else if (currentEffect == FidelityFX::UpscaleEffect::CAS_SHARPEN_ONLY || currentEffect == FidelityFX::UpscaleEffect::CAS_UPSCALING)
			{
				if (ImGui::Checkbox("Enable FidelityFX CAS Upscaling", &casUpscalingEnabled))
				{
					FidelityFX::UpscaleEffect newEffect = casUpscalingEnabled ? FidelityFX::UpscaleEffect::CAS_UPSCALING : FidelityFX::UpscaleEffect::CAS_SHARPEN_ONLY;
					lastActiveEffect = newEffect;
					UpdateScalePreset(newEffect, currentFsrMode, currentCasScalePreset);
				}
				if (casUpscalingEnabled)
				{
					const char* casLabels[] = { "Ultra Quality", "Quality", "Balanced", "Performance", "Ultra Performance" };
					int currentCasPresetInt = static_cast<int>(currentCasScalePreset);
					if (ImGui::Combo("Upscale Preset", &currentCasPresetInt, casLabels, IM_ARRAYSIZE(casLabels)))
					{
						UpdateScalePreset(FidelityFX::UpscaleEffect::CAS_UPSCALING, currentFsrMode, static_cast<FidelityFX::CASScalePreset>(currentCasPresetInt));
					}
				}
			}

			// Slider shows up when both FSR1 & RCAS is enabled or CAS (both only sharpening and sharpening & upscaling) is enabled
			if (!(currentEffect == FidelityFX::UpscaleEffect::FSR1 && !rcasEnabled)) ImGui::SliderFloat("Sharpness", &m_fidelityFX->m_sharpness, 0.0f, 1.f);
		}
	}

	ImGui::Spacing();
	ImGui::SliderFloat("Exposure", &m_exposure, 0.01f, 10.0f);
}
