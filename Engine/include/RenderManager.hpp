//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.hpp
#pragma once
#include <filesystem>
#include "Material.hpp"
#include "Window.hpp"

constexpr float EPSILON = 0.00001f;
constexpr float PI = 3.14159f;
constexpr float HALF_PI = 0.5f * PI;
constexpr int   XINDEX = 0;
constexpr int   YINDEX = 1;
constexpr int   ZINDEX = 2;

enum class MeshType
{
	PLANE,
	CUBE,
	SPHERE,
	TORUS,
	CYLINDER,
	CONE,
};

class RenderManager
{
public:
	//--------------------Common--------------------//
	virtual void BeginRender(glm::vec4 bgColor) = 0;
	virtual void EndRender() = 0;
	virtual void DeleteWithIndex() = 0;
	GraphicsMode GetGraphicsMode() { return gMode; };

	//--------------------2D Render--------------------//
	virtual void LoadTexture(const std::filesystem::path& path_, std::string name_) = 0;
	virtual void LoadQuad(glm::vec4 color_, float isTex_, float isTexel_) = 0;

	std::vector<TwoDimension::VertexUniform>* GetVertexUniforms2D() { return &vertexUniforms2D; };
	std::vector<TwoDimension::FragmentUniform>* GetFragmentUniforms2D() { return &fragUniforms2D; };

	//--------------------3D Render--------------------//
	virtual void LoadMesh(MeshType type, glm::vec4 color, int stacks, int slices) = 0;

	std::vector<ThreeDimension::VertexUniform>* GetVertexUniforms3D() { return &vertexUniforms3D; };
	std::vector<ThreeDimension::FragmentUniform>* GetFragmentUniforms3D() { return &fragUniforms3D; };
protected:
	//--------------------Common--------------------//
	GraphicsMode gMode{ GraphicsMode::GL };
	unsigned int quadCount{ 0 };
	std::vector<uint16_t> indices;

	//--------------------2D Render--------------------//
	std::vector<TwoDimension::Vertex> vertices2D;
	std::vector<TwoDimension::VertexUniform> vertexUniforms2D;
	std::vector<TwoDimension::FragmentUniform> fragUniforms2D;

	//--------------------3D Render--------------------//
	bool DegenerateTri(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
	{
		return (glm::distance(v0, v1) < EPSILON || glm::distance(v1, v2) < EPSILON || glm::distance(v2, v0) < EPSILON);
	}
	float RoundDecimal(float input) { return std::floor(input * 10000.0f + 0.5f) / 10000.0f; }
	glm::vec4 RoundDecimal(const glm::vec4& input)
	{
		return glm::vec4(RoundDecimal(input[0]), RoundDecimal(input[1]), RoundDecimal(input[2]), 1.0f);
	}

	std::vector<ThreeDimension::Vertex> vertices3D;
	std::vector<ThreeDimension::VertexUniform> vertexUniforms3D;
	std::vector<ThreeDimension::FragmentUniform> fragUniforms3D;
};
