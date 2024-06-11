//Author: JEYOON YU
//Project: CubeEngine
//File: GLVertexArray.cpp
#include "GLVertexBuffer.hpp"

GLVertexBuffer::GLVertexBuffer(GLsizei size) : size(size)
{
	glCreateBuffers(1, &vboHandle);
	glNamedBufferStorage(vboHandle, 1, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

GLVertexBuffer::~GLVertexBuffer()
{
	glDeleteBuffers(1, &vboHandle);
}

void GLVertexBuffer::SetData(const void* data)
{
	glNamedBufferSubData(vboHandle, 0, size, data);
}

//void GLVertexBuffer::Use()
//{
//	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
//	//glBindBuffer(GL_ARRAY_BUFFER, 0);
//}
