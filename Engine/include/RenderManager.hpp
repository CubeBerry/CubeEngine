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

#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLUniformBuffer.hpp"
#include "VKVertexBuffer.hpp"
#include "VKIndexBuffer.hpp"
#include "VKUniformBuffer.hpp"

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
	virtual bool BeginRender(glm::vec3 bgColor) = 0;
	virtual void EndRender() = 0;
	virtual void DeleteWithIndex(int id) = 0;
	void SetRenderType(RenderType type) { rMode = type; };
	void SetPolygonType(PolygonType type) { pMode = type; };
	GraphicsMode GetGraphicsMode() { return gMode; };
	RenderType GetRenderType() { return rMode; };

	//--------------------2D Render--------------------//
	virtual void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) = 0;

	//--------------------3D Render--------------------//
	void CreateMesh(
		std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
#ifdef _DEBUG
		std::vector<ThreeDimension::NormalVertex>& normalVertices,
#endif
		MeshType type, const std::filesystem::path& path, int stacks, int slices
	);
	void AddDirectionalLight(const ThreeDimension::DirectionalLightUniform& light)
	{
		directionalLightUniforms.push_back(light);
	}
	void DeleteDirectionalLights()
	{
		directionalLightUniforms.clear();
		directionalLightUniforms.shrink_to_fit();
	}
	void AddPointLight(const ThreeDimension::PointLightUniform& light)
	{
		pointLightUniforms.push_back(light);
	}
	void DeletePointLights()
	{
		pointLightUniforms.clear();
		pointLightUniforms.shrink_to_fit();
	}
	std::vector<ThreeDimension::DirectionalLightUniform>& GetDirectionalLightUniforms() { return directionalLightUniforms; };
	std::vector<ThreeDimension::PointLightUniform>& GetPointLightUniforms() { return pointLightUniforms; };

#ifdef _DEBUG
	void DrawNormals(bool isDraw) { this->isDrawNormals = isDraw; };
#endif

	//Skybox
	virtual void LoadSkybox(const std::filesystem::path& path) = 0;
	virtual void DeleteSkybox() = 0;
protected:
	//--------------------Common--------------------//
	GraphicsMode gMode{ GraphicsMode::GL };
	RenderType rMode = RenderType::TwoDimension;
	PolygonType pMode = PolygonType::FILL;

	//--------------------2D Render--------------------//

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

	//Lighting
	std::vector<ThreeDimension::DirectionalLightUniform> directionalLightUniforms;
	std::vector<ThreeDimension::PointLightUniform> pointLightUniforms;

#ifdef _DEBUG
	bool isDrawNormals{ false };
#endif

	//Assimp
	Assimp::Importer importer;

	//Skybox
	bool skyboxEnabled{ false };
private:
	static void BuildIndices(const std::vector<ThreeDimension::Vertex>& tempVertices, std::vector<uint32_t>& tempIndices, const unsigned int verticesCount, const int stacks, const int slices);
	//Assimp
	void ProcessNode(
		std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
		aiNode* node, const aiScene* scene, unsigned int& verticesCount, int childCount);
	void ProcessMesh(
		std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
		aiMesh* mesh, const aiScene* scene, unsigned int& verticesCount, int childCount);
	void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* mat)
{
	glm::mat4 result;

	result[0][0] = static_cast<float>(mat->a1); result[0][1] = static_cast<float>(mat->b1);  result[0][2] = static_cast<float>(mat->c1); result[0][3] = static_cast<float>(mat->d1);
	result[1][0] = static_cast<float>(mat->a2); result[1][1] = static_cast<float>(mat->b2);  result[1][2] = static_cast<float>(mat->c2); result[1][3] = static_cast<float>(mat->d2);
	result[2][0] = static_cast<float>(mat->a3); result[2][1] = static_cast<float>(mat->b3);  result[2][2] = static_cast<float>(mat->c3); result[2][3] = static_cast<float>(mat->d3);
	result[3][0] = static_cast<float>(mat->a4); result[3][1] = static_cast<float>(mat->b4);  result[3][2] = static_cast<float>(mat->c4); result[3][3] = static_cast<float>(mat->d4);

	return result;
}

// Buffer
union VertexBuffer
{
	VertexBuffer() {}
	// GL
	GLVertexBuffer* glVertexBuffer;
	// VK
	VKVertexBuffer<Vertex>* vkVertexBuffer;
#ifdef _DEBUG
	// GL
	GLVertexBuffer* glNormalVertexBuffer;
	// VK
	VKVertexBuffer<ThreeDimension::NormalVertex>* vkNormalVertexBuffer;
#endif
};

union IndexBuffer
{
	IndexBuffer() {}
	// GL
	GLIndexBuffer* glIndexBuffer;
	// VK
	VKIndexBuffer* vkIndexBuffer;
};

union VertexUniformBuffer
{
	VertexUniformBuffer() {}
	// GL
	GLUniformBuffer<VertexUniform>* glVertexUniformBuffer;
	// VK
	VKUniformBuffer<VertexUniform>* vkVertexUniformBuffer;
};

union FragmentUniformBuffer
{
	FragmentUniformBuffer() {}
	// GL
	GLUniformBuffer<FragmentUniform>* glFragmentUniformBuffer;
	// VK
	VKUniformBuffer<FragmentUniform>* vkFragmentUniformBuffer;
};

union MaterialUniformBuffer
{
	MaterialUniformBuffer() {}
	// GL
	GLUniformBuffer<ThreeDimension::Material>* glMaterialUniformBuffer;
	// VK
	VKUniformBuffer<ThreeDimension::Material>* vkMaterialUniformBuffer;
};
