//Author: JEYOON YU
//Project: CubeEngine
//File: DXComputeBuffer.cpp
#include "DXComputeBuffer.hpp"
#include "DXHelper.hpp"
#include "DXRenderTarget.hpp"

#include <d3d12.h>
#include <d3dcompiler.h>

#include <stdexcept>

void DXComputeBuffer::InitComputeBuffer(
	const ComPtr<ID3D12Device>& device,
	const std::filesystem::path& computePath,
	int width, int height,
	const ComPtr<ID3D12DescriptorHeap>& srvHeap,
	const std::unique_ptr<DXRenderTarget>& renderTarget
)
{
	m_width = width;
	m_height = height;

	// Create Root Signature
	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	// Input Texture (t0)
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	// Outpyt Texture (u0)
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameter;
	rootParameter.InitAsDescriptorTable(2, ranges);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
	computeRootSignatureDesc.Init_1_1(
		1, &rootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_NONE
	);

	ComPtr<ID3DBlob> signature, error;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&computeRootSignatureDesc, &signature, &error);
	if (FAILED(hr))
	{
		if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
		throw std::runtime_error("Failed to serialize compute root signature.");
	}

	DXHelper::ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_computeRootSignature)));
	DXHelper::ThrowIfFailed(m_computeRootSignature->SetName(L"Compute Root Signature"));

	// Create Pipeline State Object
#ifdef _DEBUG
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> errorMessages;

	ComPtr<ID3DBlob> computeShader;
	hr = D3DCompileFromFile(
		computePath.c_str(),
		nullptr,
		nullptr,
		"computeMain",
		"cs_5_1",
		compileFlags,
		0,
		&computeShader,
		&errorMessages
	);
	if (FAILED(hr))
	{
		if (errorMessages)
		{
			const char* string = static_cast<const char*>(errorMessages->GetBufferPointer());
			OutputDebugStringA("Compute Shader Compilation Error:\n");
			OutputDebugStringA(string);
		}
		throw std::runtime_error("Failed to compile compute shader.");
	}

	// Create Pipeline State Object (PSO) description
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_computeRootSignature.Get();
	psoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	// Create the Pipeline State Object (PSO)
	DXHelper::ThrowIfFailed(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_computePipelineState)));
	DXHelper::ThrowIfFailed(m_computePipelineState->SetName(L"Compute PSO"));

	//m_texture.Reset();

	auto textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_width,
		m_height,
		1, 0, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_postProcessTexture)
	));
	m_postProcessTexture->SetName(L"Post Process Output Texture");

	UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHeapStart(srvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHeapStart(srvHeap->GetGPUDescriptorHandleForHeapStart());

	constexpr int postProcessDescriptorOffset = 505;

	m_postProcessInputSrvCpuHandle = srvHeapStart;
	m_postProcessInputSrvCpuHandle.Offset(postProcessDescriptorOffset, descriptorSize);
	m_postProcessInputSrvGpuHandle = srvGpuHeapStart;
	m_postProcessInputSrvGpuHandle.Offset(postProcessDescriptorOffset, descriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
	device->CreateShaderResourceView(renderTarget->GetMSAARenderTarget().Get(), &srvDesc, m_postProcessInputSrvCpuHandle);

	m_postProcessOutputUavCpuHandle = srvHeapStart;
	m_postProcessOutputUavCpuHandle.Offset(postProcessDescriptorOffset + 1, descriptorSize);
	m_postProcessOutputUavGpuHandle = srvGpuHeapStart;
	m_postProcessOutputUavGpuHandle.Offset(postProcessDescriptorOffset + 1, descriptorSize);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(m_postProcessTexture.Get(), nullptr, &uavDesc, m_postProcessOutputUavCpuHandle);
}

void DXComputeBuffer::PostProcess(
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12DescriptorHeap>& srvHeap,
	const std::unique_ptr<DXRenderTarget>& dxRenderTarget,
	const ComPtr<ID3D12Resource>& renderTarget)
{
	commandList->SetPipelineState(m_computePipelineState.Get());
	commandList->SetComputeRootSignature(m_computeRootSignature.Get());

	CD3DX12_RESOURCE_BARRIER barriers[2];
	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(dxRenderTarget->GetMSAARenderTarget().Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ResourceBarrier(2, barriers);

	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->SetComputeRootDescriptorTable(0, m_postProcessInputSrvGpuHandle);

	UINT dispatchX = (m_width + 7) / 8;
	UINT dispatchY = (m_height + 7) / 8;
	commandList->Dispatch(dispatchX, dispatchY, 1);

	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_postProcessTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->ResourceBarrier(2, barriers);

	commandList->CopyResource(renderTarget.Get(), m_postProcessTexture.Get());

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);
}
