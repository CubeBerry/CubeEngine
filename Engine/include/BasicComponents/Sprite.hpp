//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: Sprite.hpp
#pragma once
#include "Component.hpp"
#include"../include/Object.hpp"
#include "Animation.hpp"
#include "../Material.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

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

	//Add Mesh
	void AddQuad(glm::vec4 color_);
	void AddMeshWithTexture(std::string name_, glm::vec4 color_ = { 1.f,1.f,1.f,1.f });
	void AddMeshWithTexel(std::string name_, glm::vec4 color_ = { 1.f,1.f,1.f,1.f });
	void AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f });
	void AddMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_);
	void RecreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f });
	void RecreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color, float metallic_, float roughness_);

	//Animation
	void LoadAnimation(const std::filesystem::path& spriteInfoFile, std::string name);
	glm::vec2 GetHotSpot(int index);
	glm::vec2 GetFrameSize() const { return frameSize; };

	void PlayAnimation(int anim);
	bool IsAnimationDone() { return animations[currAnim]->IsAnimationDone(); };
	int GetCurrentAnim() const { return currAnim; };
	void UpdateAnimation(float dt);

	//Getter
	int GetMaterialId() { return materialId; };
	void ChangeTexture(std::string name);
	SpriteDrawType GetSpriteDrawType() { return spriteDrawType; }
	MeshType GetMeshType() { return meshType; }
	std::filesystem::path GetModelFilePath() { return filePath; }
	int GetStacks() { return stacks; };
	int GetSlices() { return slices; };
	std::string GetTextureName() { return textureName; }
	bool GetIsTex() { return isTex; }
	Vertex GetVertex() { return vertex; }
	VertexUniform GetVertexUniform() { return vertexUniform; };
	FragmentUniform GetFragmentUniform() { return fragmentUniform; };
	glm::vec3 GetSpecularColor() { return material.specularColor; }
	float GetShininess() { return material.shininess; }
	float GetMetallic() { return material.metallic; }
	float GetRoughness() { return material.roughness; }

#ifdef _DEBUG
	ThreeDimension::NormalVertex GetNormalVertex() { return normalVertex; }
#endif

	//Setter
	void SetMaterialId(int index) { materialId = index; }
	void AddSpriteToManager();
	void SetColor(glm::vec4 color);
	glm::vec4 GetColor();
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

	int materialId = 0;
	std::string textureName = "";

	Vertex vertex;
	VertexUniform vertexUniform;
	FragmentUniform fragmentUniform;
	ThreeDimension::Material material;

#ifdef _DEBUG
	ThreeDimension::NormalVertex normalVertex;
#endif
};