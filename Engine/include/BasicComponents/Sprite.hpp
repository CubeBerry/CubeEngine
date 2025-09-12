//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: Sprite.hpp
#pragma once
#include "Component.hpp"
#include "Animation.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

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
	void AddQuadWithTexture(std::string name_, glm::vec4 color_ = { 1.f,1.f,1.f,1.f }, bool isTexel_ = false);
	// Add Mesh
	void AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f }, float metallic_ = 0.3f, float roughness_ = 0.3f);


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
	glm::vec3 GetSpecularColor(int index = 0) { return subMeshes[index].bufferWrapper->GetClassifiedData<BufferWrapper::BufferData3D>().material.specularColor; }
	float GetShininess(int index = 0) { return subMeshes[index].bufferWrapper->GetClassifiedData<BufferWrapper::BufferData3D>().material.shininess; }
	float GetMetallic(int index = 0) { return subMeshes[index].bufferWrapper->GetClassifiedData<BufferWrapper::BufferData3D>().material.metallic; }
	float GetRoughness(int index = 0) { return subMeshes[index].bufferWrapper->GetClassifiedData<BufferWrapper::BufferData3D>().material.roughness; }
	// Buffer
	std::vector<SubMesh>& GetSubMeshes() { return subMeshes; }

	//Setter
	void AddSpriteToManager();
	void SetColor(glm::vec4 color);
	void SetSpriteDrawType(SpriteDrawType type) { spriteDrawType = type; }
	void SetIsTex(bool state);
	void SetSpecularColor(glm::vec3 sColor, int index = 0) { subMeshes[index].bufferWrapper->GetClassifiedData<BufferWrapper::BufferData3D>().material.specularColor = sColor; }
	void SetShininess(float amount, int index = 0) { subMeshes[index].bufferWrapper->GetClassifiedData<BufferWrapper::BufferData3D>().material.shininess = amount; }
	void SetMetallic(float amount, int index = 0) { subMeshes[index].bufferWrapper->GetClassifiedData<BufferWrapper::BufferData3D>().material.metallic = amount; }
	void SetRoughness(float amount, int index = 0) { subMeshes[index].bufferWrapper->GetClassifiedData<BufferWrapper::BufferData3D>().material.roughness = amount; }

	//For CompFuncQueue
	void CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f }, float metallic_ = 0.3f, float roughness_ = 0.3f);
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

	// Buffer
	std::vector<SubMesh> subMeshes;
};
