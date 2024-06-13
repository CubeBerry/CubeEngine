//Author: JEYOON YU
//Project: CubeEngine
//File: GLVertexBuffer.hpp
#pragma once
#include "glew/glew.h"

class GLVertexBuffer
{
public:
	GLVertexBuffer(GLsizei size);
	~GLVertexBuffer();

	void SetData(const void* data);
	//void Use();

	GLuint GetHandle() { return vboHandle; };
private:
	GLuint vboHandle{ 0 };
	GLsizei size{ 0 };
};