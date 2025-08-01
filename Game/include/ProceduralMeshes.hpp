//Author: JEYOON YU
//Project: CubeEngine
//File: ProceduralMeshes.hpp
#pragma once
#include "GameState.hpp"
#include "RenderManager.hpp"
#include <array>

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
	int stacks{ 1 };
	int slices{ 1 };
	std::array<float, 4> color = { 1.f, 0.f, 0.f, 1.f };

	MeshType currentMesh{ MeshType::PLANE };

	//ImGui - Projection
	float cNear{ 0.001f }, cFar{ 45.f };
	float cFov{ 22.5f };
	//ImGui - Object
	std::filesystem::path objPath = "";

	float angle[2] = { 0,0 };
};
