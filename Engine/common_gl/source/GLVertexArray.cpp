//Author: JEYOON YU
//Project: CubeEngine
//File: GLVertexArray.cpp
#include "GLVertexArray.hpp"

GLVertexArray::GLVertexArray()
{
	glCreateVertexArrays(1, &vaoHandle);
}

GLVertexArray::~GLVertexArray()
{
	glDeleteVertexArrays(1, &vaoHandle);
}

void GLVertexArray::AddVertexBuffer(GLVertexBuffer&& buffer, std::initializer_list<GLAttributeLayout> layout)
{
	GLuint vboHandle = buffer.GetHandle();
	for (const GLAttributeLayout& attribute : layout)
	{
		glEnableVertexArrayAttrib(vaoHandle, attribute.vertex_layout_location);
		//bind a buffer to a vertex buffer bind point
		glVertexArrayVertexBuffer(vaoHandle, attribute.vertex_layout_location, vboHandle, attribute.offset, attribute.stride);
		glVertexArrayAttribFormat(vaoHandle, attribute.vertex_layout_location, attribute.component_dimension, attribute.component_type, attribute.normalized, attribute.relative_offset);
		glVertexArrayAttribBinding(vaoHandle, attribute.vertex_layout_location, attribute.vertex_layout_location);
	}
	buffers.push_back(std::move(buffer));
}

void GLVertexArray::SetIndexBuffer(GLIndexBuffer&& buffer)
{
	numIndices = buffer.GetCount();
	indexBuffer = std::move(buffer);
	glVertexArrayElementBuffer(vaoHandle, indexBuffer.GetHandle());
}

void GLVertexArray::Use(bool bind)
{
	if (bind == true)
	{
		glBindVertexArray(vaoHandle);
	}
	else if (bind == false)
	{
		glBindVertexArray(0);
	}
}
