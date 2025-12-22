//Author: JEYOON YU
//Project: CubeEngine
//File: DXHelper.hpp
#pragma once
#include <wrl.h>
#include <vector>
#include <filesystem>
// For CompileShader
//#include <d3dcommon.h>

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
    void ThrowIfFailed(HRESULT hr/*, UINT64& frameCounter*/);

    // Read in precompiled shader
    std::vector<char> ReadShaderFile(const std::filesystem::path& path);

    // @TODO AI-Generated: Review shader compilation function
    //ID3DBlob* CompileShader(const std::string& shaderCode, const wchar_t* entryPoint, const wchar_t* targetProfile);
}
