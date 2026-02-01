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
	m_fidelityFX = std::make_unique<FidelityFX>();
	m_fidelityFX->InitializeBackend(m_renderManager->m_device, m_renderManager->m_width, m_renderManager->m_height);
	m_fidelityFX->CreateCasContext();
	m_fidelityFX->CreateFSR1Context();
}

void DXPostProcessContext::OnResize()
{
	m_fidelityFX->OnResize(m_renderManager->m_device, m_renderManager->m_width, m_renderManager->m_height);
}

void DXPostProcessContext::Execute(ICommandListWrapper* commandListWrapper)
{
	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	auto* msaaTarget = m_renderManager->m_renderTarget->GetMSAARenderTarget().Get();
	auto* backBuffer = m_renderManager->m_renderTargets[m_renderManager->m_frameIndex].Get();

	auto preResolveBarrier = CD3DX12_RESOURCE_BARRIER::Transition(msaaTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	commandList->ResourceBarrier(1, &preResolveBarrier);

	FidelityFX::UpscaleEffect currentEffect = m_fidelityFX->GetCurrentEffect();
	const bool useUpscaling = currentEffect == FidelityFX::UpscaleEffect::FSR1 || currentEffect == FidelityFX::UpscaleEffect::CAS_UPSCALING;

	if (useUpscaling)
	{
		ComPtr<ID3D12Resource> intermediateRenderTarget = m_renderManager->m_renderTarget->GetRenderTarget();
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RESOLVE_DEST);
		commandList->ResourceBarrier(1, &barrier);
		commandList->ResolveSubresource(intermediateRenderTarget.Get(), 0, msaaTarget, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COMMON);
		commandList->ResourceBarrier(1, &barrier);
		m_fidelityFX->Execute(commandList, intermediateRenderTarget.Get(), backBuffer);
	}
	else
	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST);
		commandList->ResourceBarrier(1, &barrier);
		commandList->ResolveSubresource(backBuffer, 0, msaaTarget, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		if (currentEffect == FidelityFX::UpscaleEffect::CAS_SHARPEN_ONLY)
		{
			barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			commandList->ResourceBarrier(1, &barrier);
			m_fidelityFX->Execute(commandList, backBuffer, backBuffer);
		}
		else
		{
			auto postResolveBarrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
			commandList->ResourceBarrier(1, &postResolveBarrier);
		}
	}
}

void DXPostProcessContext::CleanUp()
{
	m_fidelityFX.reset();
	//m_lowResRenderTarget.Reset();
}

void DXPostProcessContext::UpdateScalePreset(const FidelityFX::UpscaleEffect& effect, const FfxFsr1QualityMode& mode, const FidelityFX::CASScalePreset& preset)
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
					m_fidelityFX->GetRenderHeight()
				);
			}

			return true;
		}
	);
}
