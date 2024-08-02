//Author: JEYOON YU
//Project: CubeEngine
//File: GLVertexArray.cpp
#include "GLVertexArray.hpp"

#include "glCheck.hpp"

GLVertexArray::~GLVertexArray()
{
	glCheck(glDeleteVertexArrays(1, &vaoHandle));
}

void GLVertexArray::Initialize()
{
	glCheck(glCreateVertexArrays(1, &vaoHandle));
}

void GLVertexArray::AddVertexBuffer(GLVertexBuffer&& buffer, size_t size, std::initializer_list<GLAttributeLayout> layout)
{
	//glCheck(glBindVertexArray(vaoHandle));

	GLuint vboHandle = buffer.GetHandle();
	//glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboHandle));

	//for (int index = 0; index < layout.size(); ++index)
	//{
	//	const GLAttributeLayout* attribute = layout.begin() + index;

	//	glCheck(glEnableVertexArrayAttrib(vaoHandle, index));
	//	glCheck(glVertexAttribPointer(index, attribute->component_dimension, attribute->component_type, attribute->normalized, size, reinterpret_cast<const void*>(attribute->relative_offset)));
	//}

	//glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
	//glCheck(glBindVertexArray(0));

	//bind a buffer to a vertex buffer bind point
	glCheck(glVertexArrayVertexBuffer(vaoHandle, 0, vboHandle, 0, static_cast<GLsizei>(size)));

	for (const GLAttributeLayout& attribute : layout)
	{
		glCheck(glEnableVertexArrayAttrib(vaoHandle, attribute.vertex_layout_location));
		switch (attribute.component_type)
		{
		case GL_FLOAT:
			//For Float
			glCheck(glVertexArrayAttribFormat(vaoHandle, attribute.vertex_layout_location, attribute.component_dimension, attribute.component_type, attribute.normalized, attribute.relative_offset));
			break;
		case GL_INT:
			//For Int
			glCheck(glVertexArrayAttribIFormat(vaoHandle, attribute.vertex_layout_location, attribute.component_dimension, attribute.component_type, attribute.relative_offset));
			break;
		}
		glCheck(glVertexArrayAttribBinding(vaoHandle, attribute.vertex_layout_location, 0));
	}
}

void GLVertexArray::SetIndexBuffer(GLIndexBuffer&& buffer)
{
	numIndices = buffer.GetCount();
	indexBuffer = std::move(buffer);
	glCheck(glVertexArrayElementBuffer(vaoHandle, indexBuffer.GetHandle()));
}

void GLVertexArray::Use(bool bind)
{
	if (bind == true)
	{
		glCheck(glBindVertexArray(vaoHandle));
	}
	else if (bind == false)
	{
		glCheck(glBindVertexArray(0));
	}
}
