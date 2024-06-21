//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.hpp
#pragma once
#include <filesystem>
#include <glm/vec4.hpp>
#include "Material.hpp"
#include "Window.hpp"

//template <GraphicsMode mode>
//struct TextureType;
//
//template <>
//struct TextureType<GraphicsMode::GL>
//{
//	using Type = GLTexture;
//};
//
//template <>
//struct TextureType<GraphicsMode::VK>
//{
//	using Type = VKTexture;
//};

//template <GraphicsMode mode>
class RenderManager
{
public:
	//using Texture = typename TextureType<mode>::Type;

	virtual void BeginRender() = 0;
	virtual void EndRender() = 0;

	//--------------------Texture Render--------------------//
	virtual void LoadTexture(const std::filesystem::path& path_, std::string name_) = 0;
	virtual void LoadQuad(glm::vec4 color_, float isTex_, float isTexel_) = 0;
	virtual void DeleteWithIndex() = 0;

	std::vector<VertexUniform>* GetVertexVector() { return &vertexVector; };
	std::vector<FragmentUniform>* GetFragmentVector() { return &fragVector; };
	GraphicsMode GetGraphicsMode() { return gMode; };
	//virtual void* GetTexture(std::string name) = 0;
	//Texture* GetTexture(std::string name)
	//{
	//	for (auto& tex : textures)
	//	{
	//		if (tex->GetName() == name)
	//		{
	//			return tex;
	//		}
	//	}
	//	return nullptr;
	//};
protected:
	GraphicsMode gMode{ GraphicsMode::GL };

	//--------------------Texture Render--------------------//
	unsigned int quadCount{ 0 };

	std::vector<Vertex> texVertices;
	std::vector<uint16_t> texIndices;
	std::vector<VertexUniform> vertexVector;
	std::vector<FragmentUniform> fragVector;
};
