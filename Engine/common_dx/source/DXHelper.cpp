//Author: JEYOON YU
//Project: CubeEngine
//File: DXHelper.cpp
#include "DXHelper.hpp"

#include <d3dx12/d3dx12_core.h>
#include <stdexcept>
#include <fstream>
// For CompileShader
//#include <dxcapi.h>

#include <conio.h>
#define ERROR_QUIT(value, ...) if(!(value)) { printf("ERROR: "); printf(__VA_ARGS__); printf("\nPress any key to terminate...\n"); _getch(); throw 0; }

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

// @TODO AI-Generated: Review shader compilation function
//ID3DBlob* DXHelper::CompileShader(const std::string& shaderCode, const wchar_t* entryPoint, const wchar_t* targetProfile)
//{
//    ID3DBlob* resultBlob = nullptr;
//    HMODULE sDxCompilerDLL = LoadLibrary("dxcompiler.dll");
//    ERROR_QUIT(sDxCompilerDLL, "Failed to initialize compiler.");
//    if (sDxCompilerDLL)
//    {
//        DxcCreateInstanceProc pDxcCreateInstance;
//        pDxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(sDxCompilerDLL, "DxcCreateInstance");
//
//        if (pDxcCreateInstance)
//        {
//            ComPtr<IDxcUtils> pUtils;
//            ComPtr<IDxcCompiler> pCompiler;
//            ComPtr<IDxcBlobEncoding> pSource;
//            ComPtr<IDxcOperationResult> pOperationResult;
//
//            if (SUCCEEDED(pDxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils))) && SUCCEEDED(pDxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler))))
//            {
//                if (SUCCEEDED(pUtils->CreateBlob(shaderCode.c_str(), static_cast<uint32_t>(shaderCode.length()), 0, &pSource)))
//                {
//                    if (SUCCEEDED(pCompiler->Compile(pSource.Get(), nullptr, entryPoint, targetProfile, nullptr, 0, nullptr, 0, nullptr, &pOperationResult)))
//                    {
//                        HRESULT hr;
//                        pOperationResult->GetStatus(&hr);
//                        if (SUCCEEDED(hr))
//                        {
//                            pOperationResult->GetResult((IDxcBlob**)&resultBlob);
//                        }
//                    }
//                }
//            }
//        }
//    }
//
//    ERROR_QUIT(resultBlob, "Failed to compile GWG Library.");
//    return resultBlob;
//}

// @TODO AI-Generated: Review shader compilation function
//ID3DBlob* DXHelper::CompileShader(const std::string& shaderCode, const wchar_t* entryPoint, const wchar_t* targetProfile)
//{
//    ID3DBlob* resultBlob = nullptr;
//    HMODULE sDxCompilerDLL = LoadLibrary("dxcompiler.dll");
//    ERROR_QUIT(sDxCompilerDLL, "Failed to initialize compiler.");
//    if (sDxCompilerDLL)
//    {
//        DxcCreateInstanceProc pDxcCreateInstance;
//        pDxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(sDxCompilerDLL, "DxcCreateInstance");
//
//        if (pDxcCreateInstance)
//        {
//            ComPtr<IDxcUtils> pUtils;
//            ComPtr<IDxcCompiler> pCompiler;
//            ComPtr<IDxcBlobEncoding> pSource;
//            ComPtr<IDxcOperationResult> pOperationResult;
//
//            if (SUCCEEDED(pDxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils))) && SUCCEEDED(pDxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler))))
//            {
//                if (SUCCEEDED(pUtils->CreateBlob(shaderCode.c_str(), static_cast<uint32_t>(shaderCode.length()), 0, &pSource)))
//                {
//                    HRESULT compileResult = pCompiler->Compile(pSource.Get(), nullptr, entryPoint, targetProfile, nullptr, 0, nullptr, 0, nullptr, &pOperationResult);
//
//                    if (SUCCEEDED(compileResult) && pOperationResult)
//                    {
//                        HRESULT hr;
//                        pOperationResult->GetStatus(&hr);
//
//                        // Always try to get error buffer for warnings/errors
//                        ComPtr<IDxcBlobEncoding> pErrorBlob;
//                        if (SUCCEEDED(pOperationResult->GetErrorBuffer(&pErrorBlob)) && pErrorBlob && pErrorBlob->GetBufferSize() > 0)
//                        {
//                            std::string errorMsg(static_cast<const char*>(pErrorBlob->GetBufferPointer()), pErrorBlob->GetBufferSize());
//                            OutputDebugStringA("=== Shader Compilation Output ===\n");
//                            OutputDebugStringA(errorMsg.c_str());
//                            OutputDebugStringA("\n=================================\n");
//
//                            // Also print to console
//                            printf("=== Shader Compilation Output ===\n");
//                            printf("%s\n", errorMsg.c_str());
//                            printf("=================================\n");
//                        }
//
//                        if (SUCCEEDED(hr))
//                        {
//                            pOperationResult->GetResult((IDxcBlob**)&resultBlob);
//                        }
//                        else
//                        {
//                            // Print additional context on failure
//                            printf("Shader compilation failed!\n");
//                            printf("  Entry Point: %ls\n", entryPoint);
//                            printf("  Target Profile: %ls\n", targetProfile);
//                        }
//                    }
//                    else
//                    {
//                        printf("Failed to execute shader compilation. HRESULT: 0x%08X\n", compileResult);
//                    }
//                }
//                else
//                {
//                    printf("Failed to create source blob from shader code.\n");
//                }
//            }
//            else
//            {
//                printf("Failed to create DXC Utils or Compiler instance.\n");
//            }
//        }
//        else
//        {
//            printf("Failed to get DxcCreateInstance from dxcompiler.dll.\n");
//        }
//    }
//
//    ERROR_QUIT(resultBlob, "Failed to compile GWG Library.");
//    return resultBlob;
//}
