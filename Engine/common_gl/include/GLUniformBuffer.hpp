//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: GLUniformBuffer.hpp
#pragma once
#include "glew/glew.h"
#include "glCheck.hpp"

template <typename Type>
class GLUniformBuffer
{
public:
    GLUniformBuffer() = default;
    ~GLUniformBuffer();

    void InitUniform(GLuint program, GLuint binding, const char* name, size_t size, void* data);
    void UpdateUniform(size_t size, const void* data);

    [[nodiscard]] GLuint GetHandle() const noexcept { return uniformHandle; }
private:
    GLuint uniformHandle{ 0 };
    GLuint uniformBlockIndex{ 0 };
    GLuint uniformBindingPoint{ 0 };
};

template <typename Type>
GLUniformBuffer<Type>::~GLUniformBuffer()
{
    glCheck(glDeleteBuffers(1, &uniformHandle));
}

template <typename Type>
void GLUniformBuffer<Type>::InitUniform(GLuint program, GLuint binding, const char* name, size_t size, void* /*data*/)
{
    //GLuint uniformBlockIndex = glGetUniformBlockIndex(program, name);
    //GLuint uniformBindingPoint{ binding };
    //glUniformBlockBinding(program, uniformBlockIndex, uniformBindingPoint);

    //glGenBuffers(1, &uniformHandle);
    //glBindBuffer(GL_UNIFORM_BUFFER, uniformHandle);
    //glBufferData(GL_UNIFORM_BUFFER, sizeof(vector), vector.data(), GL_STATIC_DRAW);
    //glBindBufferBase(GL_UNIFORM_BUFFER, uniformBindingPoint, uniformHandle);

    uniformBlockIndex = glCheck(glGetUniformBlockIndex(program, name));
    if (uniformBlockIndex == GL_INVALID_INDEX) 
    {        
        return;
    }
    uniformBindingPoint = binding;
    glCheck(glUniformBlockBinding(program, uniformBlockIndex, uniformBindingPoint));

    glCheck(glGenBuffers(1, &uniformHandle));
    glCheck(glBindBuffer(GL_UNIFORM_BUFFER, uniformHandle));
    glCheck(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
    //glCheck(glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data));
    //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, uniformBindingPoint, uniformHandle));
    glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

template <typename Type>
void GLUniformBuffer<Type>::UpdateUniform(size_t size, const void* data)
{
    //glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniformHandle, 0, vector.size() * sizeof(Type));

    glCheck(glBindBuffer(GL_UNIFORM_BUFFER, uniformHandle));
    // @TODO glBufferData should be used only for light uniforms
    glCheck(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
    glCheck(glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data));
    glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}
