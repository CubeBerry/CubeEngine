//Author: JEYOON YU
//Project: CubeEngine
//File: GLVertexArray.hpp
#pragma once
#include "glew/glew.h"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include <vector>

struct GLAttributeLayout
{
    enum ComponentType : GLenum
    {
        Float = GL_FLOAT,
        Int = GL_INT,
        Bool = GL_BOOL
    };
    ComponentType component_type = ComponentType::Float;
    enum NumComponents : GLint
    {
        _1 = 1,
        _2 = 2,
        _3 = 3,
        _4 = 4
    };
    NumComponents component_dimension = NumComponents::_1;
    // Is this the 1st, 2nd, 3rd... (0, 1, 2...) "in attribute" of the vertex shader?
    GLuint vertex_layout_location = 0;
    // should be false for float types
    // if normalized is GL_TRUE, then integer data is normalized to the range [-1, 1]
    // If normalized is GL_FALSE then integer data is directly converted to floating point.
    GLboolean normalized = GL_FALSE;
    // byte offset to read the very 1st attribute
    // should be 0 for parallel array and struct of arrays
    // should be offsetof(Vertex, field) for array of structs
    GLuint relative_offset = 0;
    // byte offset into where we are in the buffer
    //  do we start at the beginning or somewhere in the middle for this attribute?
    GLintptr offset = 0;
    // how many bytes to step to the next attribute
    GLsizei stride = 0;
};

class GLVertexArray
{
public:
	GLVertexArray() = default;
	~GLVertexArray();
    void Initialize();

	void AddVertexBuffer(GLVertexBuffer&& buffer, size_t size, std::initializer_list<GLAttributeLayout> layout);
    void SetIndexBuffer(GLIndexBuffer&& buffer);
    void Use(bool bind);

    [[nodiscard]] GLsizei GetVerticesCount() const
    {
        return numVertices;
    }

    [[nodiscard]] GLsizei GetIndicesCount() const noexcept
    {
        return numIndices;
    }
private:
	GLuint vaoHandle{ 0 };
    GLIndexBuffer indexBuffer;

    GLsizei numVertices{ 0 };
    GLsizei numIndices{ 0 };
};