//Author: JEYOON YU
//Project: CubeEngine
//File: DXComputeBuffer.hpp
#pragma once
#include <directx/d3dx12.h>
#include <wrl.h>

#include <filesystem>

using Microsoft::WRL::ComPtr;

class DXRenderTarget;

class DXComputeBuffer
{
public:
	DXComputeBuffer(
		/*const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12CommandQueue>& commandQueue,
		const std::vector<uint32_t>* indices*/) = default;
	~DXComputeBuffer() = default;

	DXComputeBuffer(const DXComputeBuffer&) = delete;
	DXComputeBuffer& operator=(const DXComputeBuffer&) = delete;
	DXComputeBuffer(const DXComputeBuffer&&) = delete;
	DXComputeBuffer& operator=(const DXComputeBuffer&&) = delete;

	void InitComputeBuffer(
		const ComPtr<ID3D12Device>& device,
		const std::filesystem::path& computePath,
		int width, int height,
		const ComPtr<ID3D12DescriptorHeap>& srvHeap,
		const std::unique_ptr<DXRenderTarget>& renderTarget
	);
	void PostProcess(
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		/*const ComPtr<ID3D12Device>& device,
		int width, int height,*/
		const ComPtr<ID3D12DescriptorHeap>& srvHeap,
		const std::unique_ptr<DXRenderTarget>& dxRenderTarget,
		const ComPtr<ID3D12Resource>& renderTarget);
private:
	ComPtr<ID3D12RootSignature> m_computeRootSignature;
	ComPtr<ID3D12PipelineState> m_computePipelineState;

	ComPtr<ID3D12Resource> m_postProcessTexture;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_postProcessInputSrvCpuHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_postProcessInputSrvGpuHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_postProcessOutputUavCpuHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_postProcessOutputUavGpuHandle;

	int m_width, m_height;
};
