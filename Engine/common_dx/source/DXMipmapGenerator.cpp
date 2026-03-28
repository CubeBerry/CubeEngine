//Author: JEYOON YU
//Project: CubeEngine
//File: DXMipmapGenerator.cpp
#include "DXMipmapGenerator.hpp"

#include <stdexcept>
#include <algorithm>
#include <vector>
#include <d3dcompiler.h>

DXMipmapGenerator::DXMipmapGenerator(const ComPtr<ID3D12Device>& device)
{
	// Create root signature
	CD3DX12_DESCRIPTOR_RANGE1 srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
	CD3DX12_DESCRIPTOR_RANGE1 uavRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].InitAsConstants(2, 0);
	rootParameters[1].InitAsDescriptorTable(1, &srvRange);
	rootParameters[2].InitAsDescriptorTable(1, &uavRange);

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.MipLODBias = 0.0f;
    sampler.MaxAnisotropy = 1;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> signature, error;
    if (FAILED(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error)))
    {
        throw std::runtime_error("Failed to serialize mipmap root signature.\n" + std::string((char*)error->GetBufferPointer()));
    }
    device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

	// Compile compute shader
    ComPtr<ID3DBlob> computeShader;
    HRESULT hr = D3DCompileFromFile(L"../Engine/shaders/hlsl/MipmapGen.comp.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "computeMain", "cs_5_1", 0, 0, &computeShader, &error);
    if (FAILED(hr)) throw std::runtime_error("Failed to compile MipmapGen compute shader.\n" + std::string(static_cast<char*>(error->GetBufferPointer())));

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());
    device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

	// Create descriptor heap for SRV/UAV
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 256;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptorHeap));
    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DXMipmapGenerator::Generate(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12Resource>& texture) const
{
    D3D12_RESOURCE_DESC texDesc = texture->GetDesc();

    if (texDesc.MipLevels <= 1 || !(texDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)) return;

    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetComputeRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_descriptorHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    for (uint32_t mip = 0; mip < texDesc.MipLevels - 1; ++mip)
    {
        uint32_t srcMip = mip;
		uint32_t dstMip = mip + 1;

        uint32_t dstWidth = std::max(1u, static_cast<uint32_t>(texDesc.Width >> dstMip));
        uint32_t dstHeight = std::max(1u, static_cast<uint32_t>(texDesc.Height >> dstMip));

        std::vector<D3D12_RESOURCE_BARRIER> barriers;
        for (uint32_t arraySlice = 0; arraySlice < texDesc.DepthOrArraySize; ++arraySlice)
        {
            uint32_t dstSubresource = D3D12CalcSubresource(dstMip, arraySlice, 0, texDesc.MipLevels, texDesc.DepthOrArraySize);
            barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, dstSubresource));
        }
        commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());

		// Create SRV for source mip level
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2DArray.MostDetailedMip = srcMip;
        srvDesc.Texture2DArray.MipLevels = 1;
        srvDesc.Texture2DArray.FirstArraySlice = 0;
        srvDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;

        device->CreateShaderResourceView(texture.Get(), &srvDesc, cpuHandle);
        commandList->SetComputeRootDescriptorTable(1, gpuHandle);
        cpuHandle.Offset(1, m_descriptorSize);
        gpuHandle.Offset(1, m_descriptorSize);

		// Create UAV for destination mip level
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = texDesc.Format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.MipSlice = dstMip;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;

        device->CreateUnorderedAccessView(texture.Get(), nullptr, &uavDesc, cpuHandle);
        commandList->SetComputeRootDescriptorTable(2, gpuHandle);
        cpuHandle.Offset(1, m_descriptorSize);
        gpuHandle.Offset(1, m_descriptorSize);

		float texelSize[2] = { 1.0f / dstWidth, 1.0f / dstHeight };
        commandList->SetComputeRoot32BitConstants(0, 2, texelSize, 0);

		UINT dispatchX = (dstWidth + 7) / 8;
        UINT dispatchY = (dstHeight + 7) / 8;
		commandList->Dispatch(dispatchX, dispatchY, texDesc.DepthOrArraySize);

        for (auto& b : barriers)
        {
            std::swap(b.Transition.StateBefore, b.Transition.StateAfter);
        }
        commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
    }
}
