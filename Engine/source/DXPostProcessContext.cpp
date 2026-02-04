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
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

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
	rootSignatureDesc.Init_1_1(1, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	if (FAILED(hr))
	{
		if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
		Engine::GetLogger().LogError(LogCategory::D3D12, "Failed to serialize root signature.");
	}

	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	DXHelper::ThrowIfFailed(m_rootSignature->SetName(L"Tone Mapping Root Signature"));

	DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };
	m_pipeline = std::make_unique<DXPipeLine>(
		m_renderManager->m_device,
		m_rootSignature,
		"../Engine/shaders/hlsl/ToneMapping.vert.hlsl",
		"../Engine/shaders/hlsl/ToneMapping.frag.hlsl",
		std::initializer_list<DXAttributeLayout>{},
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE,
		sampleDesc,
		CD3DX12_BLEND_DESC(D3D12_DEFAULT).RenderTarget[0],
		false,
		false,
		false,
		// RTV Format should be DXGI_FORMAT_R8G8B8A8_UNORM to render tone-mapped LDR output
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);
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
		auto barrierIRT = CD3DX12_RESOURCE_BARRIER::Transition(hdrRenderTarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COMMON);
		commandList->ResourceBarrier(1, &barrierIRT);
	}

	FidelityFX::UpscaleEffect currentEffect = m_fidelityFX->GetCurrentEffect();
	if (currentEffect != FidelityFX::UpscaleEffect::NONE)
	{
		m_fidelityFX->Execute(commandList, hdrRenderTarget.Get(), backBuffer);
	}
	else
	{
		// --------------------Apply Tone-Mapping--------------------
		D3D12_RESOURCE_BARRIER barriers[2];
		// Main Render Target: PRESENT -> RENDER_TARGET
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

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->DrawInstanced(3, 1, 0, 0);

		// HDR Render Target: PIXEL_SHADER_RESOURCE (SRV) -> COMMON
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(hdrRenderTarget.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
		commandList->ResourceBarrier(1, &barrier);
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
