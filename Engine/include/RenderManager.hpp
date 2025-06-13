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
		std::vector<ThreeDimension::Vertex> vertices;
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
		GLVertexArray vertexArray;
		GLVertexBuffer* vertexBuffer;
#ifdef _DEBUG
		GLVertexArray normalVertexArray;
		GLVertexBuffer* normalVertexBuffer;
#endif
		GLIndexBuffer* indexBuffer;
	};

	struct GLUniformBuffer2D
	{
		GLUniformBuffer<TwoDimension::VertexUniform>* vertexUniformBuffer;
		GLUniformBuffer<TwoDimension::FragmentUniform>* fragmentUniformBuffer;
	};

	struct GLUniformBuffer3D
	{
		GLUniformBuffer<ThreeDimension::VertexUniform>* vertexUniformBuffer;
		GLUniformBuffer<ThreeDimension::FragmentUniform>* fragmentUniformBuffer;
		GLUniformBuffer<ThreeDimension::Material>* materialUniformBuffer;
	};

	//--------------------Vulkan--------------------//
public:
	struct VKBuffer
	{
		VKVertexBuffer* vertexBuffer;
#ifdef _DEBUG
		VKVertexBuffer* normalVertexBuffer;
#endif
		VKIndexBuffer* indexBuffer;
	};

	//struct VKUniformBuffer2D
	//{
	//	VKUniformBuffer<TwoDimension::VertexUniform>* vertexUniformBuffer;
	//	VKUniformBuffer<TwoDimension::FragmentUniform>* fragmentUniformBuffer;
	//};

	//struct VKUniformBuffer3D
	//{
	//	VKUniformBuffer<ThreeDimension::VertexUniform>* vertexUniformBuffer;
	//	VKUniformBuffer<ThreeDimension::FragmentUniform>* fragmentUniformBuffer;
	//	VKUniformBuffer<ThreeDimension::Material>* materialUniformBuffer;
	//};

private:
	std::variant<std::monostate, GLBuffer, VKBuffer> buffer;
	std::variant<std::monostate, GLUniformBuffer2D, GLUniformBuffer3D/*, VKUniformBuffer2D, VKUniformBuffer3D*/> uniformBuffer;

public:
	BufferWrapper() : buffer(std::monostate{}), uniformBuffer(std::monostate{})
	{
		bufferData.classifiedData = std::monostate{};
	}
	// @TODO Should I use std::unique_ptr of raw pointers?
	~BufferWrapper()
	{
		std::visit([]<typename T>(T & buf)
		{
			if constexpr (!std::is_same_v<std::decay_t<T>, std::monostate>)
			{
				delete buf.vertexBuffer;
#ifdef _DEBUG
				delete buf.normalVertexBuffer;
#endif
				delete buf.indexBuffer;
			}
		}, buffer);

		std::visit([]<typename T>(T & buf)
		{
			if constexpr (!std::is_same_v<std::decay_t<T>, std::monostate>)
			{
				delete buf.vertexUniformBuffer;
				delete buf.fragmentUniformBuffer;
				if constexpr (requires { buf.materialUniformBuffer; })
				{
					delete buf.materialUniformBuffer;
				}
			}
		}, uniformBuffer);
	}

	void Initialize(GraphicsMode mode, RenderType type)
	{
		if (mode == GraphicsMode::GL)
		{
			buffer = GLBuffer{};
			if (type == RenderType::TwoDimension)
			{
				uniformBuffer = GLUniformBuffer2D{};
				bufferData.classifiedData = BufferData2D{};

				std::get<GLBuffer>(buffer).vertexArray.Initialize();
			}
			else if (type == RenderType::ThreeDimension)
			{
				uniformBuffer = GLUniformBuffer3D{};
				bufferData.classifiedData = BufferData3D{};

				std::get<GLBuffer>(buffer).vertexArray.Initialize();
#ifdef _DEBUG
				std::get<GLBuffer>(buffer).normalVertexArray.Initialize();
#endif
			}
		}
		else
		{
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

	virtual void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) = 0;
	virtual void InitializeBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) = 0;

	//--------------------3D Render--------------------//
	void CreateMesh(
		std::vector<ThreeDimension::Vertex>& vertices, std::vector<uint32_t>& indices,
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
	static void BuildIndices(const std::vector<ThreeDimension::Vertex>& tempVertices, std::vector<uint32_t>& tempIndices, const int stacks, const int slices);
	//Assimp
	void ProcessNode(
		std::vector<ThreeDimension::Vertex>& vertices, std::vector<uint32_t>& indices,
		const aiNode* node, const aiScene* scene, int childCount);
	void ProcessMesh(
		std::vector<ThreeDimension::Vertex>& vertices, std::vector<uint32_t>& indices,
		const aiMesh* mesh, const aiScene* scene, int childCount);
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
