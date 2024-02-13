//Author: JEYOON YU
//Project: CubeEngine
//File: Vertex.hpp
#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

struct Vertex
{
	alignas(16) glm::vec4 position;
	//alignas(16) glm::vec4 color;
	//glm::vec2 uv;
	alignas(4) int index;
};

struct VertexUniform
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
	alignas(16) glm::vec4 color;
	alignas(16) glm::vec4 frameSize;
	alignas(16) glm::vec4 texelPos;
	alignas(4) float isTex;
	alignas(4) float isTexel;
};

struct FragmentUniform
{
	alignas(4) int texIndex;
};