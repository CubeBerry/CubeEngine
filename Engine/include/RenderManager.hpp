//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.hpp
#pragma once
#include <filesystem>
#include "Material.hpp"
#include "Window.hpp"

class RenderManager
{
public:
	enum class MeshType
	{
		PLANE,
		CUBE,
		SPHERE,
		TORUS,
		CYLINDER,
		CONE,
	};
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
	virtual void LoadMesh(MeshType type) = 0;

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
	std::vector<ThreeDimension::Vertex> vertices3D;
	std::vector<ThreeDimension::VertexUniform> vertexUniforms3D;
	std::vector<ThreeDimension::FragmentUniform> fragUniforms3D;
};
