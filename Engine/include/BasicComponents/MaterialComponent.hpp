#pragma once
#include "Component.hpp"
#include"../include/Object.hpp"

#include <filesystem>
#include <vector>

struct Vertex;
class MaterialComponent : public Component
{
public:
	MaterialComponent() : Component(ComponentTypes::MATERIAL) {}
	~MaterialComponent() override;
	
	void Init() override;
	void Update(float dt) override;
    void End() override;

	void UpdateModel();
	void UpdateView();
	void UpdateProjection();

	void AddQuad(glm::vec4 color_, float isTex_);
	void AddQuadLine(glm::vec4 color_);
	void AddMeshWithTexture(int index);

	int GetTextureId() { return textureId; }
	void ChangeTexture(int index);
private:
	int materialId = 0;
	int textureId = 0;
};