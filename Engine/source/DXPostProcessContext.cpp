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

	auto* backBuffer = m_renderManager->m_renderTargets[m_renderManager->m_frameIndex].Get();
	ComPtr<ID3D12Resource> intermediateRenderTarget = m_renderManager->m_renderTarget->GetRenderTarget();

	// Forward Rendering (Resolve Only)
	if (!m_renderManager->m_deferredRenderingEnabled)
	{
		auto* msaaTarget = m_renderManager->m_renderTarget->GetMSAARenderTarget().Get();
		D3D12_RESOURCE_BARRIER barriers[2];
		// MSAA Target: RENDER_TARGET -> RESOLVE_SOURCE
		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(msaaTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
		// Intermediate: COMMON -> RESOLVE_DEST
		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RESOLVE_DEST);
		commandList->ResourceBarrier(2, barriers);

		// Resolve
		commandList->ResolveSubresource(intermediateRenderTarget.Get(), 0, msaaTarget, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

		// Intermediate: RESOLVE_DEST -> COMMON
		auto barrierIRT = CD3DX12_RESOURCE_BARRIER::Transition(intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COMMON);
		commandList->ResourceBarrier(1, &barrierIRT);
	}

	FidelityFX::UpscaleEffect currentEffect = m_fidelityFX->GetCurrentEffect();
	if (currentEffect != FidelityFX::UpscaleEffect::NONE)
	{
		m_fidelityFX->Execute(commandList, intermediateRenderTarget.Get(), backBuffer);
	}
	else
	{
		D3D12_RESOURCE_BARRIER barriers[2];
		// Main Render Target: PRESENT -> COPY_DEST
		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
		// Intermediate: COMMON -> COPY_SOURCE
		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
		commandList->ResourceBarrier(2, barriers);

		// Copy
		commandList->CopyResource(backBuffer, intermediateRenderTarget.Get());

		// Main Render Target: COPY_DEST -> RENDER_TARGET
		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
		// Intermediate: COPY_SOURCE -> COMMON
		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
		commandList->ResourceBarrier(2, barriers);
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
			}

			return true;
		}
	);
}
