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
	// Define Max Bones
	constexpr int MAX_BONES = 128;
	constexpr int MAX_BONE_INFLUENCE = 4;

	// Struct to hold bone offset matrix and id
	struct BoneInfo
	{
		int id;
		glm::mat4 offset;
	};

	struct alignas(16) Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		int texSubIndex{ 0 };

		int boneIDs[MAX_BONE_INFLUENCE];
		float weights[MAX_BONE_INFLUENCE];
	};

	// @TODO Should add alignas(16) for 3D pipeline but mesh pipeline does not require alignas(16) for Structured Buffer
	struct /*alignas(16)*/ QuantizedVertex
	{
		//glm::vec3 position;

		// vec2 -> uint32_t quantization (11, 11, 10)
		uint32_t position;
		glm::vec3 normal;
		glm::vec2 uv;
		int texSubIndex{ 0 };

		// Bone IDs and Weights (For simplicity in this guide, we use raw types not packed)
		glm::ivec4 boneIDs;
		glm::vec4 weights;
	};

	// For StaticSprite, Check Material.hpp ThreeDimension::DynamicQuantizedVertex for StaticQuantizedVertex
	struct StaticQuantizedVertex : public QuantizedVertex
	{
		uint32_t objectId{ 0 };
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

		// Array of final bone matrices for shader
		glm::mat4 finalBones[MAX_BONES];
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
	struct alignas(16) DirectionalLightUniform
	{
		glm::vec3 lightDirection = { 0.f, 0.f, 0.f };
		float ambientStrength{ 0.f };
		glm::vec3 lightColor = { 1.f, 1.f, 1.f };
		float specularStrength{ 0.f };
		float intensity{ 1.f };
	};

	struct alignas(16) PointLightUniform
	{
		glm::vec3 lightPosition = { 0.f, 0.f, 0.f };
		float ambientStrength{ 0.f };
		glm::vec3 lightColor = { 1.f, 1.f, 1.f };
		float specularStrength{ 0.f };
		float intensity{ 1.f };
		
		float constant = 1.0f;
		float linear = 0.09f;
		float quadratic = 0.032f;
		// Local Light Pass -> Light Volume Radius
		// CalculatePointLightRadius
		float radius = 0.f;
	};
}
