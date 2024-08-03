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
#ifdef _DEBUG
	void ImGuiDraw(float dt) override;
#endif
	void Restart() override;
	void End() override;
private:
	int stacks{ 2 };
	int slices{ 2 };
	bool isFill{ true };
	float color[4]{ 1.f, 0.f, 0.f, 1.f };

	MeshType currentMesh{ MeshType::PLANE };
};
