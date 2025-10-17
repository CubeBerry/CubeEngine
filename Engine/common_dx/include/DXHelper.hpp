//Author: JEYOON YU
//Project: CubeEngine
//File: DXHelper.hpp
#pragma once
#include <directx/d3dx12.h>
#include <stdexcept>
#include <filesystem>
#include <fstream>

#include "DebugTools.hpp"

using Microsoft::WRL::ComPtr;

namespace DXHelper
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        const char* what() const noexcept override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X",
                static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr/*, UINT64& frameCounter*/)
    {
#if USE_NSIGHT_AFTERMATH
        if (FAILED(hr))
        {
            auto tdrTerminationTimeout = std::chrono::seconds(3);
            auto tStart = std::chrono::steady_clock::now();
            auto tElapsed = std::chrono::milliseconds::zero();

            GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
            AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

            while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed &&
                status != GFSDK_Aftermath_CrashDump_Status_Finished &&
                tElapsed < tdrTerminationTimeout)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

                auto tEnd = std::chrono::steady_clock::now();
                tElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart);
            }

            if (status != GFSDK_Aftermath_CrashDump_Status_Finished)
            {
                std::stringstream err_msg;
                err_msg << "Unexpected crash dump status: " << status;
                throw std::runtime_error(err_msg.str());
            }

            exit(-1);
        }

        //frameCounter++;
#else
		if (FAILED(hr))
		{
			throw com_exception(hr);
		}
#endif
    }

    inline void CreateFenceSet(
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

    // Read in precompiled shader
    inline std::vector<char> ReadShaderFile(const std::filesystem::path& path)
    {
        std::ifstream shaderData(path, std::ios::in | std::ios::binary);
        if (!shaderData.is_open())
            throw std::runtime_error{ "File Does Not Exist" };

        shaderData.seekg(0, std::ios::end);
        size_t fileSize = shaderData.tellg();
        shaderData.seekg(0);
        std::vector<char> shaderCode(fileSize / sizeof(char));
        shaderData.read(reinterpret_cast<char*>(shaderCode.data()), fileSize);
        shaderData.close();

        return shaderCode;
    }
}
