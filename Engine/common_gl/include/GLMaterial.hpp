//Author: JEYOON YU
//Project: CubeEngine
//File: Vertex.hpp
#pragma once
#pragma warning( disable : 4324 )

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

struct alignas(16) GLVertex
{
	glm::vec4 position;
	//alignas(16) glm::vec4 color;
	//glm::vec2 uv;
	int index;
};

struct alignas(16) GLVertexUniform
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec4 color;
	glm::vec4 frameSize;
	glm::vec4 texelPos;
	float isTex;
	float isTexel;
};

struct alignas(16) GLFragmentUniform
{
	int texIndex;
};