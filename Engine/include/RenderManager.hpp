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
	virtual void BeginRender(glm::vec4 bgColor) = 0;
	virtual void EndRender() = 0;

	//--------------------Texture Render--------------------//
	virtual void LoadTexture(const std::filesystem::path& path_, std::string name_) = 0;
	virtual void LoadQuad(glm::vec4 color_, float isTex_, float isTexel_) = 0;
	virtual void DeleteWithIndex() = 0;

	std::vector<TwoDimension::VertexUniform>* GetVertexVector() { return &vertexVector; };
	std::vector<TwoDimension::FragmentUniform>* GetFragmentVector() { return &fragVector; };
	GraphicsMode GetGraphicsMode() { return gMode; };
protected:
	GraphicsMode gMode{ GraphicsMode::GL };

	//--------------------Texture Render--------------------//
	unsigned int quadCount{ 0 };

	std::vector<TwoDimension::Vertex> texVertices;
	std::vector<uint16_t> texIndices;
	std::vector<TwoDimension::VertexUniform> vertexVector;
	std::vector<TwoDimension::FragmentUniform> fragVector;
};
