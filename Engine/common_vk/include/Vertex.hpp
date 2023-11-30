#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

struct alignas(16) Vertex
{
	glm::vec4 position;
	glm::vec4 color;
	//glm::vec2 uv;
	float index;
	float isTex;
};

struct alignas(16) UniformMatrix
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	float texIndex;
};