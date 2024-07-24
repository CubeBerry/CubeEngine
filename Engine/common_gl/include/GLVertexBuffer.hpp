//Author: JEYOON YU
//Project: CubeEngine
//File: GLVertexBuffer.hpp
#pragma once
#include "glew/glew.h"

class GLVertexBuffer
{
public:
	GLVertexBuffer();
	~GLVertexBuffer();

	void SetData(GLsizei size, const void* data);
	//void Use();

	GLuint GetHandle() { return vboHandle; };
private:
	GLuint vboHandle{ 0 };
};