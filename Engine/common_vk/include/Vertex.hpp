#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

struct Vertex
{
	glm::vec4 position;
	//glm::vec3 color;
	//glm::vec2 uv;
	float index;
};

struct UniformMatrix
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};