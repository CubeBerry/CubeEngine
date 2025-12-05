//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: DynamicSprite.hpp
#pragma once
#include "ISprite.hpp"

class DynamicSprite : public ISprite
{
public:
	DynamicSprite() : ISprite() { Init(); spriteType = SpriteType::DYNAMIC; };
	~DynamicSprite() override;

	void Init() override;
	void Update(float dt) override;
	void End() override;

	//Update Matrices
	void UpdateModel(glm::vec3 pos_, glm::vec3 size_, float angle) override;
	void UpdateModel(glm::vec3 pos_, glm::vec3 size_, glm::vec3 angle) override;
	void UpdateView() override;
	void UpdateProjection() override;

	// Add Quad
	void AddQuad(glm::vec4 color_);
	void AddQuadWithTexture(std::string name_, glm::vec4 color_ = { 1.f,1.f,1.f,1.f }, bool isTexel_ = false);

	// Animation
	void LoadAnimation(const std::filesystem::path& spriteInfoFile, std::string name);
	glm::vec2 GetHotSpot(int index);
	glm::vec2 GetFrameSize() const { return frameSize; };

	void PlayAnimation(int anim);
	bool IsAnimationDone() { return animations[currAnim]->IsAnimationDone(); };
	int GetCurrentAnim() const { return currAnim; };
	void UpdateAnimation(float dt);

	//For CompFuncQueue
	void CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f }, float metallic_ = 0.3f, float roughness_ = 0.3f) override;
private:
	//Animation
	glm::vec2 GetFrameTexel(int frameNum) const;
	glm::vec2 textureSize;
	glm::vec2 frameSize;
	std::vector<glm::vec2> frameTexel;
	std::vector<glm::vec2> hotSpotList;

	int currAnim;
	std::vector<Animation*> animations;
};
