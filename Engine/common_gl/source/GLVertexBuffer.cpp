//Author: JEYOON YU
//Project: CubeEngine
//File: GLVertexBuffer.cpp
#include "GLVertexBuffer.hpp"

#include "glCheck.hpp"

GLVertexBuffer::GLVertexBuffer(GLsizei size) : size(size)
{
	glCheck(glCreateBuffers(1, &vboHandle));
	glCheck(glNamedBufferStorage(vboHandle, size, nullptr, GL_DYNAMIC_STORAGE_BIT));
}

GLVertexBuffer::~GLVertexBuffer()
{
	glCheck(glDeleteBuffers(1, &vboHandle));
}

void GLVertexBuffer::SetData(const void* data)
{
	glCheck(glNamedBufferSubData(vboHandle, 0, size, data));
}

//void GLVertexBuffer::Use()
//{
//	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
//	//glBindBuffer(GL_ARRAY_BUFFER, 0);
//}
