//Author: JEYOON YU
//Project: CubeEngine
//File: ProceduralMeshes.cpp
#include "ProceduralMeshes.hpp"
#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

void ProceduralMeshes::Init()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::ThreeDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 1.f);
	Engine::GetCameraManager().SetFar(45.f);
	Engine::GetCameraManager().SetBaseFov(22.5f);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,0.f,3.f });
	//Engine::GetCameraManager().SetCameraPosition(glm::vec3{ 2.f, 2.f, 2.f });
	//Engine::GetCameraManager().SetCenter(glm::vec3{ 0.f, 0.f, 0.f });

	currentMesh = MeshType::PLANE;
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	//Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::CUBE, "", 10, 10, { 1.0, 0.0, 0.0, 1.0 });
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/obj/teapot.obj", 10, 10, { 1.0, 0.0, 0.0, 1.0 });

	ThreeDimension::VertexLightingUniform l;
	l.lightPosition = glm::vec4(0.f, 15.f, 3.f, 1.f);
	l.lightColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
	l.viewPosition = glm::vec4(0.f, 0.f, 3.f, 1.f);
	l.ambientStrength = 0.5f;
	l.specularStrength = 0.5f;
	l.isLighting = true;
	Engine::GetRenderManager()->EnableLighting(true, l);
}

void ProceduralMeshes::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);

	//if (Engine::GetObjectManager().GetLastObject() != nullptr)
	//	Engine::GetObjectManager().Destroy(Engine::GetObjectManager().GetLastObject()->GetId());

	//switch (currentMesh)
	//{
	//case MeshType::PLANE:
	//	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
	//	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	//	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::PLANE, stacks, slices, { color[0], color[1], color[2], color[3] });
	//	break;
	//case MeshType::CUBE:
	//	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
	//	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	//	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::CUBE, stacks, slices, { color[0], color[1], color[2], color[3] });
	//	break;
	//case MeshType::SPHERE:
	//	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
	//	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	//	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::SPHERE, stacks, slices, { color[0], color[1], color[2], color[3] });
	//	break;
	//default:
	//	break;
	//}

	if (isFill)
		Engine::GetRenderManager()->SetPolygonType(PolygonType::FILL);
	else
		Engine::GetRenderManager()->SetPolygonType(PolygonType::LINE);
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
		if (ImGui::MenuItem("Sphere", "2"))
		{
			stacks = 30;
			slices = 30;
			currentMesh = MeshType::SPHERE;
		}
		ImGui::EndMenu();
	}

	switch (currentMesh)
	{
	case MeshType::PLANE:
		if (ImGui::SliderInt("stacks", &stacks, 2, 30))
		{
		}
		if (ImGui::SliderInt("slices", &slices, 2, 30))
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
	case MeshType::SPHERE:
		if (ImGui::SliderInt("stacks", &stacks, 5, 35))
		{
		}
		if (ImGui::SliderInt("slices", &slices, 5, 35))
		{
		}
		break;
	}

	if (ImGui::Button("FILL", ImVec2(100, 0)))
	{
		isFill = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("LINE", ImVec2(100, 0)))
	{
		isFill = false;
	}

	ImGui::ColorPicker4("Mesh Color", color);
	ImGui::End();
}
#endif

void ProceduralMeshes::Restart()
{
	Engine::GetObjectManager().DestroyAllObjects();
}

void ProceduralMeshes::End()
{
	isFill = true;
	Engine::GetRenderManager()->SetPolygonType(PolygonType::FILL);

	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}
