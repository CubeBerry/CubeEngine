//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.hpp
#pragma once
#define NOMINMAX

#include <filesystem>
#include <variant>
#include "Material.hpp"
#include "Window.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "GLVertexArray.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLUniformBuffer.hpp"
#include "VKVertexBuffer.hpp"
#include "VKIndexBuffer.hpp"
#include "DXVertexBuffer.hpp"
#include "DXIndexBuffer.hpp"
#include "DXConstantBuffer.hpp"

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

// Buffer
struct BufferWrapper
{
	//--------------------Common--------------------//
public:
	struct BufferData2D
	{
		std::vector<TwoDimension::Vertex> vertices;
		TwoDimension::VertexUniform vertexUniform;
		TwoDimension::FragmentUniform fragmentUniform;
	};

	struct BufferData3D
	{
		std::vector<ThreeDimension::QuantizedVertex> vertices;
#ifdef _DEBUG
		std::vector<ThreeDimension::NormalVertex> normalVertices;
#endif
		ThreeDimension::VertexUniform vertexUniform;
		ThreeDimension::FragmentUniform fragmentUniform;
		ThreeDimension::Material material;
	};
private:
	struct BufferData
	{
		std::vector<uint32_t> indices;
		std::variant<std::monostate, BufferData2D, BufferData3D> classifiedData;
	} bufferData;

	//--------------------OpenGL--------------------//
public:
	struct GLBuffer
	{
		std::unique_ptr<GLVertexArray> vertexArray;
		std::unique_ptr<GLVertexBuffer> vertexBuffer;
#ifdef _DEBUG
		std::unique_ptr<GLVertexArray> normalVertexArray;
		std::unique_ptr<GLVertexBuffer> normalVertexBuffer;
#endif
		std::unique_ptr<GLIndexBuffer> indexBuffer;
	};

	struct GLUniformBuffer2D
	{
		std::unique_ptr<GLUniformBuffer<TwoDimension::VertexUniform>> vertexUniformBuffer;
		std::unique_ptr<GLUniformBuffer<TwoDimension::FragmentUniform>> fragmentUniformBuffer;
	};

	struct GLUniformBuffer3D
	{
		std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>> vertexUniformBuffer;
		std::unique_ptr<GLUniformBuffer<ThreeDimension::FragmentUniform>> fragmentUniformBuffer;
		std::unique_ptr<GLUniformBuffer<ThreeDimension::Material>> materialUniformBuffer;
	};

	//--------------------Vulkan--------------------//
	struct VKBuffer
	{
		std::unique_ptr<VKVertexBuffer> vertexBuffer;
#ifdef _DEBUG
		std::unique_ptr<VKVertexBuffer> normalVertexBuffer;
#endif
		std::unique_ptr<VKIndexBuffer> indexBuffer;
	};

	//--------------------DirectX--------------------//
	struct DXBuffer
	{
		std::unique_ptr<DXVertexBuffer> vertexBuffer;
#ifdef _DEBUG
		std::unique_ptr<DXVertexBuffer> normalVertexBuffer;
#endif
		std::unique_ptr<DXIndexBuffer> indexBuffer;
	};

	struct DXConstantBuffer2D
	{
		std::unique_ptr<DXConstantBuffer<TwoDimension::VertexUniform>> vertexUniformBuffer;
		std::unique_ptr<DXConstantBuffer<TwoDimension::FragmentUniform>> fragmentUniformBuffer;
	};

	struct DXConstantBuffer3D
	{
		std::unique_ptr<DXConstantBuffer<ThreeDimension::VertexUniform>> vertexUniformBuffer;
		std::unique_ptr<DXConstantBuffer<ThreeDimension::FragmentUniform>> fragmentUniformBuffer;
		std::unique_ptr<DXConstantBuffer<ThreeDimension::Material>> materialUniformBuffer;
	};

private:
	std::variant<std::monostate, GLBuffer, VKBuffer, DXBuffer> buffer;
	std::variant<std::monostate, GLUniformBuffer2D, GLUniformBuffer3D, /*, VKUniformBuffer2D, VKUniformBuffer3D*/ DXConstantBuffer2D, DXConstantBuffer3D> uniformBuffer;

public:
	BufferWrapper() : buffer(std::monostate{}), uniformBuffer(std::monostate{})
	{
		bufferData.classifiedData = std::monostate{};
	}
	~BufferWrapper() = default;

	BufferWrapper(const BufferWrapper&) = delete;
	BufferWrapper& operator=(const BufferWrapper&) = delete;
	BufferWrapper(BufferWrapper&&) = default;
	BufferWrapper& operator=(BufferWrapper&&) = default;

	void Initialize(GraphicsMode mode, RenderType type)
	{
		switch(mode)
		{
			case GraphicsMode::GL:
				buffer = GLBuffer{};
				if (type == RenderType::TwoDimension)
				{
					uniformBuffer = GLUniformBuffer2D{};
					bufferData.classifiedData = BufferData2D{};

					std::get<GLBuffer>(buffer).vertexArray = std::make_unique<GLVertexArray>();
					std::get<GLBuffer>(buffer).vertexArray->Initialize();
				}
				else if (type == RenderType::ThreeDimension)
				{
					uniformBuffer = GLUniformBuffer3D{};
					bufferData.classifiedData = BufferData3D{};

					std::get<GLBuffer>(buffer).vertexArray = std::make_unique<GLVertexArray>();
					std::get<GLBuffer>(buffer).vertexArray->Initialize();
#ifdef _DEBUG
					std::get<GLBuffer>(buffer).normalVertexArray = std::make_unique<GLVertexArray>();
					std::get<GLBuffer>(buffer).normalVertexArray->Initialize();
#endif
				}
				break;
			case GraphicsMode::VK:
				buffer = VKBuffer{};
				if (type == RenderType::TwoDimension)
				{
					//uniformBuffer = VKUniformBuffer2D{};
					bufferData.classifiedData = BufferData2D{};
				}
				else if (type == RenderType::ThreeDimension)
				{
					//uniformBuffer = VKUniformBuffer3D{};
					bufferData.classifiedData = BufferData3D{};
				}
				break;
			case GraphicsMode::DX:
				buffer = DXBuffer{};
				if (type == RenderType::TwoDimension)
				{
					uniformBuffer = DXConstantBuffer2D{};
					bufferData.classifiedData = BufferData2D{};
				}
				else if (type == RenderType::ThreeDimension)
				{
					uniformBuffer = DXConstantBuffer3D{};
					bufferData.classifiedData = BufferData3D{};
				}
				break;
		}
	}

	// Getter
	//--------------------Common--------------------//
	[[nodiscard]] std::vector<uint32_t>& GetIndices() noexcept { return bufferData.indices; }
	// ex) T = BufferData::BufferData2D
	template <typename T>
	[[nodiscard]] T& GetClassifiedData() noexcept { return std::get<T>(bufferData.classifiedData); }
	// ex) T = GLBuffer
	template <typename T>
	[[nodiscard]] T& GetBuffer() noexcept { return std::get<T>(buffer); }
	// ex) T = GLUniformBuffer2D
	template <typename T>
	[[nodiscard]] T& GetUniformBuffer() noexcept { return std::get<T>(uniformBuffer); }
};

struct SubMesh
{
	BufferWrapper bufferWrapper;
	ThreeDimension::Material material;
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
	virtual void InitializeBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) = 0;

	//--------------------2D Render--------------------//
	glm::mat4 CreateMesh(std::vector<TwoDimension::Vertex>& quantizedVertices);

	//--------------------3D Render--------------------//
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
	static void BuildIndices(const std::vector<ThreeDimension::Vertex>& tempVertices, std::vector<uint32_t>& tempIndices, const int stacks, const int slices);
	//Assimp
	void ProcessNode(
		std::vector<SubMesh>& subMeshes,
		const aiNode* node, const aiScene* scene, int childCount,
		glm::vec4 color, float metallic, float roughness);
	void ProcessMesh(
		std::vector<SubMesh>& subMeshes,
		const aiMesh* mesh, const aiScene* scene, int childCount,
		glm::vec4 color, float metallic, float roughness);
	void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	glm::mat4 Quantize(
		std::vector<ThreeDimension::QuantizedVertex>& quantizedVertices,
		const std::vector<ThreeDimension::Vertex>& vertices,
		glm::vec3 largestBBoxSize);
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
