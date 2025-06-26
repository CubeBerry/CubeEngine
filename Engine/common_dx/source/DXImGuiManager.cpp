//Author: JEYOON YU
//Project: CubeEngine
//File: DXImGuiManager.cpp
#include "DXImGuiManager.hpp"
#include "DXHelper.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_dx12.h"

#include <stdexcept>

DXImGuiManager::DXImGuiManager(
	SDL_Window* window,
	const ComPtr<ID3D12Device>& device,
	int numFramesInFlight
)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui_ImplSDL3_InitForD3D(window);

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DXHelper::ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

	ImGui_ImplDX12_Init(device.Get(), numFramesInFlight, DXGI_FORMAT_R8G8B8A8_UNORM, m_srvHeap.Get(),
		m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_srvHeap->GetGPUDescriptorHandleForHeapStart()
		);
}

DXImGuiManager::~DXImGuiManager()
{
	Shutdown();
}

void DXImGuiManager::Begin()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void DXImGuiManager::End(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	ImGui::Render();

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
}

void DXImGuiManager::Shutdown()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}
