//Author: JEYOON YU
//Project: CubeEngine
//File: DXInitializer.cpp
#include "DXInitializer.hpp"
#include "DXHelper.hpp"

ComPtr<ID3D12Resource> DXInitializer::CreateBufferResource(
	ComPtr<ID3D12Device> device,
	UINT64 bufferSize,
	D3D12_RESOURCE_FLAGS resourceFlags,
	D3D12_HEAP_TYPE heapType,
	D3D12_RESOURCE_STATES initialResourceState
)
{
	ComPtr<ID3D12Resource> resource;

	CD3DX12_HEAP_PROPERTIES heapProperties(heapType);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, resourceFlags);
	DXHelper::ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialResourceState, NULL, IID_PPV_ARGS(&resource)));

	return resource;
}

void DXInitializer::CreateFenceSet(
	const ComPtr<ID3D12Device>& device,
	std::wstring& targetName,
	ComPtr<ID3D12CommandAllocator>& commandAllocator,
	ComPtr<ID3D12GraphicsCommandList>& commandList,
	ComPtr<ID3D12Fence>& fence,
	HANDLE& fenceEvent
)
{
	// Create Command Allocator
	DXHelper::ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	DXHelper::ThrowIfFailed(commandAllocator->SetName((targetName + L" Command Allocator").c_str()));

	// Create Command List
	DXHelper::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
	DXHelper::ThrowIfFailed(commandList->SetName((targetName + L" Command List").c_str()));
#if USE_NSIGHT_AFTERMATH
	AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_CreateContextHandle(commandList.Get(), &hAftermathCommandListContext));
#endif
	DXHelper::ThrowIfFailed(commandList->Close());

	// Create Fence
	DXHelper::ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	DXHelper::ThrowIfFailed(fence->SetName((targetName + L" Fence").c_str()));

	// Create Fence Event
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		throw std::runtime_error("Failed to create texture fence event.");
	}
}