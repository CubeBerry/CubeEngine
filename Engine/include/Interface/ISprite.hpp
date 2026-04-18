//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: ISprite.hpp
#pragma once
#include "IComponent.hpp"
#include "BufferWrapper.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

enum class SpriteDrawType
{
	TwoDimension,
	ThreeDimension,
	UI
};

enum class MeshType;

class ISprite : public IComponent
{
public:
	ISprite() : IComponent(ComponentTypes::SPRITE) {};

	//Update Matrices
	virtual void UpdateModel(glm::vec3 pos_, glm::vec3 size_, float angle) = 0;
	virtual void UpdateModel(glm::vec3 pos_, glm::vec3 size_, glm::vec3 angle) = 0;
	virtual void UpdateView() = 0;
	virtual void UpdateProjection() = 0;

	// Add Mesh
	void AddQuad(glm::vec4 color_);
	void AddQuadWithTexture(std::string name_, glm::vec4 color_ = { 1.f,1.f,1.f,1.f }, bool isTexel_ = false);
	void LoadAnimation(const std::filesystem::path& spriteInfoFile, std::string name);
	void AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f }, float metallic_ = 0.3f, float roughness_ = 0.3f);

	// Getter
	glm::vec4 GetColor();
	MeshType GetMeshType() const { return meshType; }
	std::filesystem::path GetModelFilePath() { return filePath; }
	int GetStacks() const { return stacks; };
	int GetSlices() const { return slices; };
	std::string GetTextureName() { return textureName; }
	bool GetIsTex() const { return isTex; }
	glm::vec3 GetSpecularColor(int index = 0) { return subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.specularColor; }
	float GetShininess(int index = 0) { return subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.shininess; }
	float GetMetallic(int index = 0) { return subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.metallic; }
	float GetRoughness(int index = 0) { return subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.roughness; }
	// Buffer
	std::vector<SubMesh>& GetSubMeshes() { return subMeshes; }

	//Setter
	void SetColor(glm::vec4 color);
	void SetSpriteDrawType(SpriteDrawType type) { spriteDrawType = type; }
	void ChangeTexture(std::string name);
	void SetIsTex(bool state);
	void SetSpecularColor(glm::vec3 sColor, int index = 0) { subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.specularColor = sColor; }
	void SetShininess(float amount, int index = 0) { subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.shininess = amount; }
	void SetMetallic(float amount, int index = 0) { subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.metallic = amount; }
	void SetRoughness(float amount, int index = 0) { subMeshes[index]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.roughness = amount; }

	//For CompFuncQueue
	virtual void CreateQuad(glm::vec4 color_) = 0;
	virtual void CreateQuadWithTexture(std::string name_, glm::vec4 color_ = { 1.f,1.f,1.f,1.f }, bool isTexel_ = false) = 0;
	virtual void LoadAnimationData(const std::filesystem::path& spriteInfoFile, std::string name) = 0;
	virtual void CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f }, float metallic_ = 0.3f, float roughness_ = 0.3f) = 0;

	// BoneInfoMap
	std::map<std::string, ThreeDimension::BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCount; }
protected:
	SpriteType spriteType = SpriteType::DYNAMIC;

	SpriteDrawType spriteDrawType = SpriteDrawType::TwoDimension;

	//For RecreateMesh
	int stacks = 1;
	int slices = 1;
	bool isTex = false;
	MeshType meshType; 
	std::filesystem::path filePath = "";

	std::string textureName = "";

	// Buffer
	std::vector<SubMesh> subMeshes;

	// Store bone information extracted from Assimp
	std::map<std::string, ThreeDimension::BoneInfo> m_BoneInfoMap;
	int m_BoneCount = 0;
};
