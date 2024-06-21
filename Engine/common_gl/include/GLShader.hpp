//Author: JEYOON YU
//Project: CubeEngine
//File: GLShader.hpp
#pragma once
#include "glew/glew.h"
#include <filesystem>

class GLShader
{
public:
    enum Type : GLenum
    {
        VERTEX = GL_VERTEX_SHADER,
        FRAGMENT = GL_FRAGMENT_SHADER,
        GEOMETRY = GL_GEOMETRY_SHADER,
        TESSELLATION_CONTROL = GL_TESS_CONTROL_SHADER,
        TESSELLATION_EVALUATION = GL_TESS_EVALUATION_SHADER,
        COMPUTE = GL_COMPUTE_SHADER
    };
public:
    GLShader() = default;
    ~GLShader();

    void Use(bool bind = true) const noexcept;

    void LoadShader(const std::initializer_list<std::pair<GLShader::Type, std::filesystem::path>>& paths);

    GLuint GetProgramHandle() { return programHandle; };
private:
    GLuint programHandle{ 0 };
};