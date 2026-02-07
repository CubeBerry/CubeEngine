//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.hpp
#pragma once
#define NOMINMAX

#include <filesystem>
#include <variant>
#include <functional>
#include "Window.hpp"
#include "Utility.hpp"
#include "Interface/ISprite.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "FidelityFX.hpp"

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
	RenderManager() = default;
	RenderManager(const RenderManager&) = delete;
	RenderManager& operator=(const RenderManager&) = delete;
	RenderManager(const RenderManager&&) = delete;
	RenderManager& operator=(const RenderManager&&) = delete;

	virtual ~RenderManager() = default;

	//--------------------Common--------------------//
	virtual bool BeginRender(glm::vec3 bgColor) = 0;
	virtual void EndRender() = 0;
	virtual void ClearTextures() = 0;
	void SetRenderType(RenderType type) { rMode = type; }
	void SetPolygonType(PolygonType type) { pMode = type; }
	GraphicsMode GetGraphicsMode() const { return gMode; }
	RenderType GetRenderType() const { return rMode; }

	virtual void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) = 0;
	// @TODO Change BufferWrapper& -> BufferWrapper*
	virtual void InitializeDynamicBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) = 0;

	// FidelityFX CAS
	virtual void UpdateScalePreset(const FidelityFX::UpscaleEffect& effect, const FfxFsr1QualityMode& mode, const FidelityFX::CASScalePreset& preset) = 0;

	//--------------------2D Render--------------------//
	glm::mat4 CreateMesh(std::vector<TwoDimension::Vertex>& quantizedVertices);

	//--------------------3D Render--------------------//
	// Deferred Deletion
	void ProcessFunctionQueue()
	{
		std::erase_if(functionQueue, [](const auto& function)
			{
				return function();
			});

		// If not C++ 20
		//auto it = functionQueue.begin();
		//while (it != functionQueue.end())
		//{
		//	if ((*it)()) it = functionQueue.erase(it);
		//	else ++it;
		//}
	}

	void CreateMesh(
		std::vector<SubMesh>& subMeshes,
		MeshType type, const std::filesystem::path& path, int stacks, int slices,
		glm::vec4 color, float metallic, float roughness
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
	std::vector<ThreeDimension::DirectionalLightUniform>& GetDirectionalLightUniforms() { return directionalLightUniforms; }
	std::vector<ThreeDimension::PointLightUniform>& GetPointLightUniforms() { return pointLightUniforms; }
	// @TODO Maybe this function should be located in Light class
	// @TODO This does not match with attenuation method in shader
	//static float CalculatePointLightRadius(const glm::vec3& lightColor, float intensity, float constant, float linear, float quadratic);

	void RenderingControllerForImGui();

	//Skybox
	virtual void LoadSkybox(const std::filesystem::path& path) = 0;
	virtual void DeleteSkybox() = 0;
protected:
	//--------------------Common--------------------//
	// Graphics Mode
	GraphicsMode gMode{ GraphicsMode::GL };
	// Render Mode
	RenderType rMode = RenderType::TwoDimension;
	// Polygon Mode
	PolygonType pMode = PolygonType::FILL;

	//--------------------2D Render--------------------//

	//--------------------3D Render--------------------//
	// Deferred Deletion
	void QueueDeferredFunction(std::function<bool()>&& func)
	{
		functionQueue.push_back(std::move(func));
	}

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
	bool m_normalVectorVisualization{ false };
#endif

	//Assimp
	Assimp::Importer m_importer;

	//Skybox
	bool m_skyboxEnabled{ false };

	// Mesh Shader
	bool m_meshShaderEnabled{ false };
	unsigned int m_meshletVisualization{ 0 };

	// Work Graphs
	bool m_workGraphsEnabled{ false };
	bool m_meshNodesEnabled{ false };

	// Deferred Rendering
	bool m_deferredRenderingEnabled{ true };
private:
	static void BuildIndices(const std::vector<ThreeDimension::Vertex>& tempVertices, std::vector<uint32_t>& tempIndices, const int stacks, const int slices);
	//Assimp
	void ProcessNode(
		std::vector<SubMesh>& subMeshes,
		const aiNode* node, const aiScene* scene, int childCount,
		glm::vec3 size, glm::vec3 center, float unitScale,
		glm::vec4 color, float metallic, float roughness);
	void ProcessMesh(
		std::vector<SubMesh>& subMeshes,
		const aiMesh* mesh, const aiScene* scene, int childCount,
		glm::vec3 size, glm::vec3 center, float unitScale,
		glm::vec4 color, float metallic, float roughness);
	void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	glm::mat4 Quantize(
		std::vector<ThreeDimension::QuantizedVertex>& quantizedVertices,
		const std::vector<ThreeDimension::Vertex>& vertices,
		glm::vec3 largestBBoxSize);
	
	// Deferred Deletion
	std::vector<std::function<bool()>> functionQueue;
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
