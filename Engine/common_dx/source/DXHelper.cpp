//Author: JEYOON YU
//Project: CubeEngine
//File: DXHelper.cpp
#include "DXHelper.hpp"

#include <d3dx12/d3dx12_core.h>
#include <stdexcept>
#include <fstream>

void DXHelper::ThrowIfFailed(HRESULT hr/*, UINT64& frameCounter*/)
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

std::vector<char> DXHelper::ReadShaderFile(const std::filesystem::path& path)
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
