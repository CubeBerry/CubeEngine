//Author: JEYOON YU
//Project: CubeEngine
//File: Material.hpp
#pragma once
#pragma warning( disable : 4324 )

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

//2D
namespace TwoDimension
{
	struct alignas(16) Vertex
	{
		glm::vec4 position;
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
}

//3D
namespace ThreeDimension
{
	struct alignas(16) Vertex
	{
		glm::vec4 position;
		glm::vec4 normal;
		glm::vec2 uv;
		int index;
	};

	struct alignas(16) VertexUniform
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 color;
	};

	struct alignas(16) FragmentUniform
	{
		int texIndex;
	};
}
