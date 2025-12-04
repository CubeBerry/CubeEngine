//Author: JEYOON YU
//Project: CubeEngine
//File: DXInitializer.hpp
#pragma once
#include <d3dx12/d3dx12.h>
#include <wrl.h>

#include <string>

using Microsoft::WRL::ComPtr;

namespace DXInitializer
{
	ComPtr<ID3D12Resource> CreateBufferResource(
		ComPtr<ID3D12Device> device,
		UINT64 bufferSize,
		D3D12_RESOURCE_FLAGS resourceFlags,
		D3D12_HEAP_TYPE heapType,
		D3D12_RESOURCE_STATES initialResourceState
	);

    void CreateFenceSet(
        const ComPtr<ID3D12Device>& device,
        std::wstring& targetName,
        ComPtr<ID3D12CommandAllocator>& commandAllocator,
        ComPtr<ID3D12GraphicsCommandList>& commandList,
        ComPtr<ID3D12Fence>& fence,
        HANDLE& fenceEvent
    );
}
