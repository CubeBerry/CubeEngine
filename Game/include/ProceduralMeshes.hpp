//Author: JEYOON YU
//Project: CubeEngine
//File: ProceduralMeshes.hpp
#pragma once
#include "GameState.hpp"
#include "RenderManager.hpp"

class ProceduralMeshes : public GameState
{
public:
	ProceduralMeshes() = default;
	~ProceduralMeshes() override {}

	void Init() override;
	void Update(float dt) override;
	void ImGuiDraw(float dt) override;
	void Restart() override;
	void End() override;
private:
	int stacks{ 2 };
	int slices{ 2 };
	bool isFill{ true };
	float color[4]{ 1.f, 0.f, 0.f, 1.f };

	ThreeDimension::VertexLightingUniform l;

	MeshType currentMesh{ MeshType::PLANE };

	bool isRecreate{ false };
	void RecreateMesh();

	//ImGui - Projection
	float cNear{ 0.05f }, cFar{ 45.f };
	float cFov{ 22.5f };
	//ImGui - Object
	std::filesystem::path objPath = "";
};
