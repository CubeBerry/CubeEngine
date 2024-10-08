//Author: JEYOON YU
//Project: CubeEngine
//File: GLIndexBuffer.hpp
#pragma once
#include "glew/glew.h"
#include <vector>

class GLIndexBuffer
{
public:
    GLIndexBuffer() = default;
    GLIndexBuffer(std::vector<uint16_t>* indices);
    ~GLIndexBuffer();

    //void Use(bool bind);

    [[nodiscard]] GLuint GetHandle() const noexcept
    {
        return indexHandle;
    }

    [[nodiscard]] GLsizei GetCount() const noexcept
    {
        return count;
    }
private:
    GLuint indexHandle{ 0 };
    GLsizei count{ 0 };
};