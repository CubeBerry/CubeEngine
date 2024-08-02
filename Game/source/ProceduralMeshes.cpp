//Author: JEYOON YU
//Project: CubeEngine
//File: ProceduralMeshes.cpp
#include "ProceduralMeshes.hpp"
#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

void ProceduralMeshes::Init()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::ThreeDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 45.f);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);
	//Engine::GetCameraManager().SetCameraPosition(glm::vec3{ 2.f, 2.f, 2.f });
	//Engine::GetCameraManager().SetCenter(glm::vec3{ 0.f, 0.f, 0.f });

	currentMesh = MeshType::PLANE;
	//Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
	//Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	//Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::CUBE, 36, 36, { 1.0, 0.0, 0.0, 1.0 });
}

void ProceduralMeshes::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);

	if (Engine::GetObjectManager().GetLastObject() != nullptr)
		Engine::GetObjectManager().Destroy(Engine::GetObjectManager().GetLastObject()->GetId());

	switch (currentMesh)
	{
	case MeshType::PLANE:
		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::PLANE, stacks, slices, { 1.0, 0.0, 0.0, 1.0 });
		break;
	case MeshType::CUBE:
		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::CUBE, stacks, slices, { 1.0, 0.0, 0.0, 1.0 });
		break;
	default:
		break;
	}
}

#ifdef _DEBUG
void ProceduralMeshes::ImGuiDraw(float /*dt*/)
{
	Engine::GetGameStateManager().StateChanger();

	ImGui::Begin("Procedural Meshes", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	if (ImGui::BeginMenu("Select Mesh"))
	{
		if (ImGui::MenuItem("Plane", "0"))
		{
			stacks = 2;
			slices = 2;
			currentMesh = MeshType::PLANE;
		}
		if (ImGui::MenuItem("Cube", "1"))
		{
			stacks = 2;
			slices = 2;
			currentMesh = MeshType::CUBE;
		}
		ImGui::EndMenu();
	}

	switch (currentMesh)
	{
	case MeshType::PLANE:
		if (ImGui::SliderInt("stacks", &stacks, 2, 50))
		{
		}
		if (ImGui::SliderInt("slices", &slices, 2, 50))
		{
		}
		break;
	case MeshType::CUBE:
		if (ImGui::SliderInt("stacks", &stacks, 2, 10))
		{
		}
		if (ImGui::SliderInt("slices", &slices, 2, 10))
		{
		}
		break;
	}
	ImGui::End();
}
#endif

void ProceduralMeshes::Restart()
{
	Engine::GetObjectManager().DestroyAllObjects();
}

void ProceduralMeshes::End()
{
	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}
