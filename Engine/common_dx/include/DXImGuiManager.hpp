//Author: JEYOON YU
//Project: CubeEngine
//File: DXImGuiManager.hpp
#pragma once
#include <directx/d3dx12.h>

struct SDL_Window;

using Microsoft::WRL::ComPtr;

class DXImGuiManager
{
public:
	DXImGuiManager(
		SDL_Window* window,
		const ComPtr<ID3D12Device>& device,
		int numFramesInFlight
		);
	~DXImGuiManager();

	//void FeedEvent(const SDL_Event& event_);
	void Begin();
	void End(const ComPtr<ID3D12GraphicsCommandList>& commandList);
	void Shutdown();
private:
	// Descriptor Heap for the ImGui font texture's Shader Resource View (SRV)
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
};