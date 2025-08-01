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
		//glm::vec2 position;

		// vec2 -> uint32_t quantization (16, 16)
		uint32_t position;
	};

	struct alignas(16) VertexUniform
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 decode;
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
		int texSubIndex{ 0 };
	};

	struct alignas(16) QuantizedVertex
	{
		//glm::vec3 position;

		// vec2 -> uint32_t quantization (11, 11, 10)
		uint32_t position;
		glm::vec3 normal;
		glm::vec2 uv;
		int texSubIndex{ 0 };
	};

#ifdef _DEBUG
	struct alignas(16) NormalVertex
	{
		glm::vec3 position;
	};
#endif

	struct alignas(16) VertexUniform
	{
		glm::mat4 model;
		// @TODO move to push constants later
		glm::mat4 transposeInverseModel;
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 decode;
		glm::vec4 color;
		// @TODO move to push constants later
		glm::vec3 viewPosition;
	};

	struct alignas(16) FragmentUniform
	{
		int isTex{ 0 };
		int texIndex;
	};

	struct alignas(16) Material
	{
		glm::vec3 specularColor = glm::vec3(1.f);
		float shininess = 32.f;

		float metallic = 0.3f;
		float roughness = 0.3f;
	};

	//Lighting
	struct alignas(16) PointLightUniform
	{
		glm::vec3 lightPosition = { 0.f, 0.f, 0.f };
		float ambientStrength;
		glm::vec3 lightColor = { 1.f, 1.f, 1.f };
		float specularStrength;

		float constant = 1.0f;
		float linear = 0.09f;
		float quadratic = 0.032f;
	};

	struct alignas(16) DirectionalLightUniform
	{
		glm::vec3 lightDirection = { 0.f, 0.f, 0.f };
		float ambientStrength;
		glm::vec3 lightColor = { 1.f, 1.f, 1.f };
		float specularStrength;
	};
}
