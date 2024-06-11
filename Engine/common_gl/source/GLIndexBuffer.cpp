//Author: JEYOON YU
//Project: CubeEngine
//File: GLIndexBuffer.cpp
#include "GLIndexBuffer.hpp"

GLIndexBuffer::GLIndexBuffer(std::vector<uint16_t>* indices) : count(indices->size())
{
	glCreateBuffers(1, &indexHandle);
	glNamedBufferStorage(indexHandle, sizeof(indices), indices->data(), GL_DYNAMIC_STORAGE_BIT);
}

GLIndexBuffer::~GLIndexBuffer()
{
}

//void GLVertexBuffer::Use()
//{
//	glBindBuffer(GL_ARRAY_BUFFER, indexHandle);
//	//glBindBuffer(GL_ARRAY_BUFFER, 0);
//}
