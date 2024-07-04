//Author: JEYOON YU
//Project: CubeEngine
//File: GLUniformBuffer.hpp
#pragma once
#include "glew/glew.h"
#include <vector>
#include <iostream>

#include "../include/Material.hpp"
template <typename Material>
class GLUniformBuffer
{
public:
    GLUniformBuffer() = default;
    ~GLUniformBuffer();

    void InitUniform(GLuint program, GLuint binding, const char* name, std::vector<Material>& vector);
    void UpdateUniform(std::vector<Material>& vector);

private:
    GLuint uniformHandle{ 0 };
    GLuint uniformBlockIndex{ 0 };
    GLuint uniformBindingPoint{ 0 };
};

template <typename Material>
void GLUniformBuffer<Material>::InitUniform(GLuint program, GLuint binding, const char* name, std::vector<Material>& vector)
{
    //GLuint uniformBlockIndex = glGetUniformBlockIndex(program, name);
    //GLuint uniformBindingPoint{ binding };
    //glUniformBlockBinding(program, uniformBlockIndex, uniformBindingPoint);

    //glGenBuffers(1, &uniformHandle);
    //glBindBuffer(GL_UNIFORM_BUFFER, uniformHandle);
    //glBufferData(GL_UNIFORM_BUFFER, sizeof(vector), vector.data(), GL_STATIC_DRAW);
    //glBindBufferBase(GL_UNIFORM_BUFFER, uniformBindingPoint, uniformHandle);

    uniformBlockIndex = glGetUniformBlockIndex(program, name);
    if (uniformBlockIndex == GL_INVALID_INDEX) 
    {        
        return;
    }
    uniformBindingPoint = binding;
    glUniformBlockBinding(program, uniformBlockIndex, uniformBindingPoint);

    glGenBuffers(1, &uniformHandle);
    glBindBuffer(GL_UNIFORM_BUFFER, uniformHandle);
    glBufferData(GL_UNIFORM_BUFFER, vector.size() * sizeof(Material), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, vector.size() * sizeof(Material), vector.data());
    glBindBufferBase(GL_UNIFORM_BUFFER, uniformBindingPoint, uniformHandle);
}

template <typename Material>
void GLUniformBuffer<Material>::UpdateUniform(std::vector<Material>& vector)
{
    //glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniformHandle, 0, vector.size() * sizeof(Material));

    glBindBuffer(GL_UNIFORM_BUFFER, uniformHandle);
    glBufferData(GL_UNIFORM_BUFFER, vector.size() * sizeof(Material), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, vector.size() * sizeof(Material), vector.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
