//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: Sprite.hpp
#pragma once
#include "Component.hpp"
#include"../include/Object.hpp"
#include "Animation.hpp"

#include <filesystem>
#include <vector>

struct Vertex;

class Sprite : public Component
{
public:
	Sprite() : Component(ComponentTypes::SPRITE) {};
	~Sprite() override;
	
	void Init() override;
	void Update(float dt) override;
    void End() override;

	//Update Matrices
	void UpdateModel(glm::vec3 pos_, glm::vec3 size_, float angle);
	void UpdateView();
	void UpdateProjection();

	//Add Mesh
	void AddQuad(glm::vec4 color_);
	void AddQuadLine(glm::vec4 color_);
	void AddMeshWithTexture(int index);
	void AddMeshWithTexel(int index);

	//Animation
	void LoadAnimation(const std::filesystem::path& spriteInfoFile, int index);
	glm::vec2 GetHotSpot(int index);
	glm::vec2 GetFrameSize() const { return frameSize; };

	void PlayAnimation(int anim);
	bool IsAnimationDone() { return animations[currAnim]->IsAnimationDone(); };
	int GetCurrentAnim() const { return currAnim; };

	//Getter
	int GetTextureId() { return textureId; };
	void ChangeTexture(int index);
private:
	//Animation
	glm::vec2 GetFrameTexel(int frameNum) const;

	glm::vec2 textureSize;
	glm::vec2 frameSize;
	std::vector<glm::vec2> frameTexel;
	std::vector<glm::vec2> hotSpotList;

	int currAnim;
	std::vector<Animation*> animations;

	int materialId = 0;
	int textureId = 0;
};