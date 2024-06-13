//Author: JEYOON YU
//Project: CubeEngine
//File: GLUniformBuffer.hpp
#pragma once
#include "glew/glew.h"
#include <vector>

template <typename Material>
class GLUniformBuffer
{
public:
	GLUniformBuffer() = default;
	~GLUniformBuffer();

	void SendUniform(GLuint program, GLuint binding, const char* name, std::vector<Material> vector);
private:
	GLuint uniformHandle{ 0 };

	GLuint uniformBlockIndex{ 0 };
	GLuint uniformBindingPoint{ 0 };
};

template <typename Material>
void GLUniformBuffer<Material>::SendUniform(GLuint program, GLuint binding, const char* name, std::vector<Material> vector)
{
	GLuint uniformBlockIndex = glGetUniformBlockIndex(program, name);
	GLuint uniformBindingPoint{ binding };
	glUniformBlockBinding(program, uniformBlockIndex, uniformBindingPoint);

	glGenBuffers(1, &uniformHandle);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformHandle);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vector), vector.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, uniformBindingPoint, uniformHandle);
};