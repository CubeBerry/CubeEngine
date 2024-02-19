//Author: JEYOON YU
//Project: CubeEngine
//File: Vertex.hpp
#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

struct alignas(16) Vertex
{
	glm::vec4 position;
	//alignas(16) glm::vec4 color;
	//glm::vec2 uv;
	int index;
};

struct alignas(16) VertexUniform
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

struct alignas(16) FragmentUniform
{
	int texIndex;
};