//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: StaticSprite.hpp
#pragma once
#include "ISprite.hpp"

class StaticSprite : public ISprite
{
public:
	struct MeshNodeRecord
	{
		uint32_t objectID;
		uint32_t meshletOffset;
		uint32_t meshletCount;
		uint32_t vertexIndexOffset;
		uint32_t primitiveIndexOffset;
	};

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
	void InitializeBuffers();

	// Getter
	const std::vector<MeshNodeRecord>& GetMeshNodeRecords() const { return m_meshNodeRecords; }
	uint32_t GetGlobalTransformIndex() const { return m_globalTransformIndex; }
	const glm::mat4& GetModelMatrix() const { return m_modelMat; }

	// Setter
	void AddMeshNodeRecord(const MeshNodeRecord& record) { m_meshNodeRecords.push_back(record); }
	void ClearMeshNodeRecords() { m_meshNodeRecords.clear(); }
	void SetGlobalTransformIndex(uint32_t index) { m_globalTransformIndex = index; }
private:
	std::vector<MeshNodeRecord> m_meshNodeRecords;
	uint32_t m_globalTransformIndex{ 0 };
	glm::mat4 m_modelMat{ 1.f };
};
