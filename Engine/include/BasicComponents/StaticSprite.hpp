//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: StaticSprite.hpp
#pragma once
#include "ISprite.hpp"

class StaticSprite : public ISprite
{
public:
	StaticSprite() : ISprite() { Init(); spriteType = SpriteType::STATIC; };
	~StaticSprite() override;

	void Init() override;
	void Update(float dt) override;
	void End() override;

	//Update Matrices
	void UpdateModel(glm::vec3 pos_, glm::vec3 size_, float angle) override;
	void UpdateModel(glm::vec3 pos_, glm::vec3 size_, glm::vec3 angle) override;
	void UpdateView() override;
	void UpdateProjection() override;

	//For CompFuncQueue
	void CreateMesh3D(MeshType type, const std::filesystem::path& path, int stacks_, int slices_, glm::vec4 color = { 1.f,1.f,1.f,1.f }, float metallic_ = 0.3f, float roughness_ = 0.3f) override;
private:
};
