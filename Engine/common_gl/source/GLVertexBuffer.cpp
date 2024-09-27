//Author: JEYOON YU
//Project: CubeEngine
//File: GLVertexBuffer.cpp
#include "GLVertexBuffer.hpp"

#include "glCheck.hpp"

GLVertexBuffer::GLVertexBuffer()
{
	glCheck(glCreateBuffers(1, &vboHandle));
	//glNamedBufferStorage == immutable, glNamedBufferData == mutable
	//glCheck(glNamedBufferStorage(vboHandle, size, nullptr, GL_DYNAMIC_STORAGE_BIT));
	//glCheck(glNamedBufferData(vboHandle, 0, nullptr, GL_DYNAMIC_DRAW));
}

GLVertexBuffer::~GLVertexBuffer()
{
	glCheck(glDeleteBuffers(1, &vboHandle));
}

void GLVertexBuffer::SetData(GLsizei size, const void* data)
{
	//Sub == When the size is fixed
	//glCheck(glNamedBufferSubData(vboHandle, 0, size, data));
	glCheck(glNamedBufferData(vboHandle, size, data, GL_DYNAMIC_DRAW));
	//glCheck(glNamedBufferStorage(vboHandle, size, data, GL_DYNAMIC_STORAGE_BIT));
}

//void GLVertexBuffer::Use()
//{
//	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
//	//glBindBuffer(GL_ARRAY_BUFFER, 0);
//}
