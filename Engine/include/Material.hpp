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
		glm::vec3 position;
		int index;
	};

	struct alignas(16) VertexUniform
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 color;
		glm::vec3 frameSize;
		float isTex;
		glm::vec3 texelPos;
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
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		int index;
	};

#ifdef _DEBUG
	struct alignas(16) NormalVertex
	{
		glm::vec3 position;
		glm::vec4 color;
		int index;
	};
#endif

	struct alignas(16) VertexUniform
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 color;
	};

	struct alignas(16) VertexLightingUniform
	{
		glm::vec3 lightPosition;
		float ambientStrength;
		glm::vec3 lightColor;
		float specularStrength;
		glm::vec3 viewPosition;
		float isLighting;
	};

	struct alignas(16) FragmentUniform
	{
		int texIndex;
	};

	struct alignas(16) Material
	{
		glm::vec3 specularColor;
		float shininess;
	};
}
