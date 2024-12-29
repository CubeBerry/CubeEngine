//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.hpp
#pragma once
#define NOMINMAX

#include <filesystem>
#include "Material.hpp"
#include "Window.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

constexpr float EPSILON = 0.00001f;
constexpr float PI = 3.14159f;
constexpr float HALF_PI = 0.5f * PI;
constexpr int   XINDEX = 0;
constexpr int   YINDEX = 1;
constexpr int   ZINDEX = 2;

enum class RenderType
{
	TwoDimension,
	ThreeDimension,
};

enum class PolygonType
{
	FILL,
	LINE,
};

enum class MeshType
{
	PLANE,
	CUBE,
	SPHERE,
	TORUS,
	CYLINDER,
	CONE,
	OBJ,
};

class RenderManager
{
public:
	//--------------------Common--------------------//
	virtual void BeginRender(glm::vec3 bgColor) = 0;
	virtual void EndRender() = 0;
	virtual void DeleteWithIndex(int id) = 0;
	void SetRenderType(RenderType type) { rMode = type; };
	void SetPolygonType(PolygonType type) { pMode = type; };
	GraphicsMode GetGraphicsMode() { return gMode; };
	RenderType GetRenderType() { return rMode; };

	//--------------------2D Render--------------------//
	virtual void LoadTexture(const std::filesystem::path& path_, std::string name_) = 0;
	virtual void LoadQuad(glm::vec4 color_, float isTex_, float isTexel_) = 0;

	std::vector<TwoDimension::VertexUniform>* GetVertexUniforms2D() { return &vertexUniforms2D; };
	std::vector<TwoDimension::FragmentUniform>* GetFragmentUniforms2D() { return &fragUniforms2D; };

	//--------------------3D Render--------------------//
	virtual void LoadMesh(MeshType type, const std::filesystem::path& path, glm::vec4 color, int stacks, int slices, float shininess = 32.f, glm::vec3 specularColor = glm::vec3(1.f)) = 0;
	//void EnableLighting(bool isEnabled) { isLighting = isEnabled; };
	void AddLight(const ThreeDimension::FragmentLightingUniform& light)
	{
		fragmentLightingUniforms.push_back(light);
	}
	void DeleteLights()
	{
		fragmentLightingUniforms.clear();
		fragmentLightingUniforms.shrink_to_fit();
	}
	std::vector<ThreeDimension::FragmentLightingUniform>& GetLightingUniforms() { return fragmentLightingUniforms; };

	std::vector<ThreeDimension::VertexUniform>* GetVertexUniforms3D() { return &vertexUniforms3D; };
	std::vector<ThreeDimension::FragmentUniform>* GetFragmentUniforms3D() { return &fragUniforms3D; };

#ifdef _DEBUG
	void DrawNormals(bool isDraw) { this->isDrawNormals = isDraw; };
#endif

	//Skybox
	virtual void LoadSkyBox(
		const std::filesystem::path& right,
		const std::filesystem::path& left,
		const std::filesystem::path& top,
		const std::filesystem::path& bottom,
		const std::filesystem::path& front,
		const std::filesystem::path& back
	) = 0;
	virtual void DeleteSkyBox() = 0;
protected:
	//--------------------Common--------------------//
	GraphicsMode gMode{ GraphicsMode::GL };
	RenderType rMode = RenderType::TwoDimension;
	PolygonType pMode = PolygonType::FILL;
	unsigned int quadCount{ 0 };
	std::vector<uint32_t> indices;

	//--------------------2D Render--------------------//
	std::vector<TwoDimension::Vertex> vertices2D;
	std::vector<TwoDimension::VertexUniform> vertexUniforms2D;
	std::vector<TwoDimension::FragmentUniform> fragUniforms2D;

	//--------------------3D Render--------------------//
	static bool DegenerateTri(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
	{
		return (glm::distance(v0, v1) < EPSILON || glm::distance(v1, v2) < EPSILON || glm::distance(v2, v0) < EPSILON);
	}
	static float RoundDecimal(float input) { return std::floor(input * 10000.0f + 0.5f) / 10000.0f; }
	static glm::vec3 RoundDecimal(const glm::vec3& input)
	{
		return { RoundDecimal(input[0]), RoundDecimal(input[1]), RoundDecimal(input[2]) };
	}

	void CreateMesh(MeshType type, const std::filesystem::path& path, int stacks, int slices);

	//Lighting
	//bool isLighting{ false };
	//ThreeDimension::VertexLightingUniform vertexLightingUniform{};
	std::vector<ThreeDimension::FragmentLightingUniform> fragmentLightingUniforms;

	std::vector<ThreeDimension::Vertex> vertices3D;
	std::vector<ThreeDimension::VertexUniform> vertexUniforms3D;
	std::vector<ThreeDimension::FragmentUniform> fragUniforms3D;
	std::vector<ThreeDimension::Material> fragMaterialUniforms3D;

#ifdef _DEBUG
	bool isDrawNormals{ false };
	std::vector<ThreeDimension::NormalVertex> normalVertices3D;
	std::vector<unsigned int> normalVerticesPerMesh;
#endif

	std::vector<unsigned int> verticesPerMesh;
	std::vector<unsigned int> indicesPerMesh;

	//Assimp
	Assimp::Importer importer;

	//Skybox
	bool skyboxEnabled{ false };
private:
	static void BuildIndices(const std::vector<ThreeDimension::Vertex>& tempVertices, std::vector<uint32_t>& tempIndices, const unsigned int verticesCount, const int stacks, const int slices);
};
