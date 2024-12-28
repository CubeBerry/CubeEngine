//Author: JEYOON YU
//Project: CubeEngine
//File: GLIndexBuffer.cpp
#include "GLIndexBuffer.hpp"

#include "glCheck.hpp"

GLIndexBuffer::GLIndexBuffer(std::vector<uint32_t>* indices) : count(static_cast<GLsizei>(indices->size()))
{
	glCheck(glCreateBuffers(1, &indexHandle));
	//glCheck(glNamedBufferStorage(indexHandle, sizeof(uint32_t) * count, indices->data(), GL_DYNAMIC_STORAGE_BIT));
	glCheck(glNamedBufferData(indexHandle, sizeof(uint32_t) * count, indices->data(), GL_DYNAMIC_DRAW));
}

GLIndexBuffer::~GLIndexBuffer()
{
	glCheck(glDeleteBuffers(1, &indexHandle));
}

//void GLVertexBuffer::Use()
//{
//	glBindBuffer(GL_ARRAY_BUFFER, indexHandle);
//	//glBindBuffer(GL_ARRAY_BUFFER, 0);
//}
