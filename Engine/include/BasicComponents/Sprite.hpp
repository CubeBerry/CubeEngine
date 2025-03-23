//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: Sprite.hpp
#pragma once
#include "Component.hpp"
#include "Animation.hpp"
#include "../Material.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "GLVertexArray.hpp"
#include "RenderManager.hpp"

enum class SpriteDrawType
{
	TwoDimension,
	ThreeDimension,
	UI
};

enum class MeshType;
class Sprite : public Component
{
public:
	Sprite() : Component(ComponentTypes::SPRITE) { Init(); };
	~Sprite() override;
	
	void Init() override;
	void Update(float dt) override;
    void End() override;

	//Update Matrices
	void UpdateModel(glm::vec3 pos_, glm::vec3 size_, float angle);
	void UpdateModel(glm::vec3 pos_, glm::vec3 size_, glm::vec3 angle);
	void UpdateView();
	void UpdateProjection();

	// Add Quad
	void AddQuad(glm::vec4 color_);
	void AddQuadWithTexture(std::string name_, glm::vec4 color_ = { 1.f,1.f,1.f,1.f });
	void AddQuadWithTexel(std::string name_, glm::vec4 color_ = { 1.f,1.f,1.f,1.f });
	// Add Mesh
	void AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f });
	void AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_);
	//void RecreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f });
	//void RecreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_);

	// Animation
	void LoadAnimation(const std::filesystem::path& spriteInfoFile, std::string name);
	glm::vec2 GetHotSpot(int index);
	glm::vec2 GetFrameSize() const { return frameSize; };

	void PlayAnimation(int anim);
	bool IsAnimationDone() { return animations[currAnim]->IsAnimationDone(); };
	int GetCurrentAnim() const { return currAnim; };
	void UpdateAnimation(float dt);

	// Getter
	glm::vec4 GetColor();
	void ChangeTexture(std::string name);
	MeshType GetMeshType() const { return meshType; }
	std::filesystem::path GetModelFilePath() { return filePath; }
	int GetStacks() const { return stacks; };
	int GetSlices() const { return slices; };
	std::string GetTextureName() { return textureName; }
	bool GetIsTex() const { return isTex; }
	glm::vec3 GetSpecularColor() const { return material.specularColor; }
	float GetShininess() const { return material.shininess; }
	float GetMetallic() const { return material.metallic; }
	float GetRoughness() const { return material.roughness; }
	 
	// Buffer Data
	std::vector<Vertex>& GetVertices() { return vertices; }
#ifdef _DEBUG
	std::vector<ThreeDimension::NormalVertex>& GetNormalVertices() { return normalVertices; }
#endif
	std::vector<uint32_t>& GetIndices() { return indices; }
	VertexUniform& GetVertexUniform() { return vertexUniform; }
	FragmentUniform& GetFragmentUniform() { return fragmentUniform; }
	ThreeDimension::Material& GetMaterial() { return material; }
	// Buffer
	GLVertexArray* GetVertexArray() { return &vertexArray; };
#ifdef _DEBUG
	GLVertexArray* GetNormalVertexArray() { return &normalVertexArray; };
#endif
	VertexBufferWrapper* GetVertexBuffer() { return &vertexBuffer; }
	IndexBufferWrapper* GetIndexBuffer() { return &indexBuffer; }
	VertexUniformBufferWrapper* GetVertexUniformBuffer() { return &vertexUniformBuffer; }
	FragmentUniformBufferWrapper* GetFragmentUniformBuffer() { return &fragmentUniformBuffer; }
	MaterialUniformBufferWrapper* GetMaterialUniformBuffer() { return &materialUniformBuffer; }

	//Setter
	void AddSpriteToManager();
	void SetColor(glm::vec4 color);
	void SetSpriteDrawType(SpriteDrawType type) { spriteDrawType = type; }
	void SetIsTex(bool state);
	void SetSpecularColor(glm::vec3 sColor) { material.specularColor = sColor; }
	void SetShininess(float amount) { material.shininess = amount; }
	void SetMetallic(float amount) { material.metallic = amount; }
	void SetRoughness(float amount) { material.roughness = amount; }

	//For CompFuncQueue
	void CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f });
	void CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_);
	void DeleteFromSpriteManagerList();
	//For CompFuncQueue
private:
	//Animation
	glm::vec2 GetFrameTexel(int frameNum) const;
	glm::vec2 textureSize;
	glm::vec2 frameSize;
	std::vector<glm::vec2> frameTexel;
	std::vector<glm::vec2> hotSpotList;

	int currAnim;
	std::vector<Animation*> animations;
	SpriteDrawType spriteDrawType = SpriteDrawType::TwoDimension;

	//For RecreateMesh
	int stacks = 1;
	int slices = 1;
	bool isTex = false;
	MeshType meshType; 
	std::filesystem::path filePath = "";
	//For RecreateMesh

	std::string textureName = "";

	// Buffer Data
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
#ifdef _DEBUG
	std::vector<ThreeDimension::NormalVertex> normalVertices;
#endif
	VertexUniform vertexUniform;
	FragmentUniform fragmentUniform;
	ThreeDimension::Material material;

	// Buffer
	GLVertexArray vertexArray;
#ifdef _DEBUG
	GLVertexArray normalVertexArray;
#endif
	VertexBufferWrapper vertexBuffer;
	IndexBufferWrapper indexBuffer;
	VertexUniformBufferWrapper vertexUniformBuffer;
	FragmentUniformBufferWrapper fragmentUniformBuffer;
	MaterialUniformBufferWrapper materialUniformBuffer;
};
