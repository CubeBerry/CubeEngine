//Author: JEYOON YU
//Project: CubeEngine
//File: DXHelper.hpp
#pragma once
#include <directx/d3dx12.h>
#include <filesystem>
#include <fstream>

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
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }

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
