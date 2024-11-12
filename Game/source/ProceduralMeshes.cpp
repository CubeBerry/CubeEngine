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
	Engine::GetCameraManager().SetFar(cFar);
	Engine::GetCameraManager().SetBaseFov(cFov);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,0.f,10.f });
	//Engine::GetCameraManager().SetCameraPosition(glm::vec3{ 2.f, 2.f, 2.f });
	//Engine::GetCameraManager().SetCenter(glm::vec3{ 0.f, 0.f, 0.f });

	currentMesh = MeshType::PLANE;
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::PLANE, "", 1, 1, { 1.0, 0.0, 0.0, 1.0 });

	l.lightPosition = glm::vec4(0.f, 15.f, 10.f, 1.f);
	l.lightColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
	//viewPosition == cameraPosition
	l.viewPosition = glm::vec4(Engine::GetCameraManager().GetCameraPosition(), 1.f);
	l.ambientStrength = 0.1f;
	l.specularStrength = 0.5f;
	l.isLighting = true;
	Engine::GetRenderManager()->EnableLighting(true);
}

void ProceduralMeshes::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);

	if (isRecreate)
	{
		RecreateMesh();
		isRecreate = false;
	}

	Engine::GetRenderManager()->DrawNormals(isDrawNormals);

	//Update Color
	(*Engine::GetRenderManager()->GetVertexUniforms3D())[0].color = glm::vec4{ color[0], color[1], color[2], color[3] };

	//Update Lighting Variables
	l.viewPosition = glm::vec4(Engine::GetCameraManager().GetCameraPosition(), 1.f);
	Engine::GetRenderManager()->UpdateLighting(l.lightPosition, l.lightColor, l.viewPosition, l.ambientStrength, l.specularStrength);

	//Camera Update
	Engine::GetCameraManager().SetNear(cNear);
	Engine::GetCameraManager().SetFar(cFar);
	Engine::GetCameraManager().SetBaseFov(cFov);

	if (isFill)
		Engine::GetRenderManager()->SetPolygonType(PolygonType::FILL);
	else
		Engine::GetRenderManager()->SetPolygonType(PolygonType::LINE);
}

void ProceduralMeshes::ImGuiDraw(float /*dt*/)
{
	Engine::GetGameStateManager().StateChanger();

	ImGui::Begin("Procedural Meshes", nullptr);

	//Projection
	if (ImGui::CollapsingHeader("Projection"))
	{
		//Change Scale
		ImGui::DragFloat("FOV degrees", &cFov, 0.05f);

		//Change Scale
		ImGui::DragFloat("Near Plane", &cNear, 0.05f);

		//Change Scale
		ImGui::DragFloat("Far Plane", &cFar, 0.05f);
	}

	//Mesh
	if (ImGui::CollapsingHeader("Mesh"))
	{
		if (ImGui::BeginMenu("Select Mesh"))
		{
			if (ImGui::MenuItem("Plane", "0"))
			{
				stacks = 2;
				slices = 2;
				currentMesh = MeshType::PLANE;
				isRecreate = true;
			}
			if (ImGui::MenuItem("Cube", "1"))
			{
				stacks = 2;
				slices = 2;
				currentMesh = MeshType::CUBE;
				isRecreate = true;
			}
			if (ImGui::MenuItem("Sphere", "2"))
			{
				stacks = 30;
				slices = 30;
				currentMesh = MeshType::SPHERE;
				isRecreate = true;
			}
			if (ImGui::MenuItem("Cone", "3"))
			{
				stacks = 10;
				slices = 10;
				currentMesh = MeshType::CONE;
				isRecreate = true;
			}
			ImGui::EndMenu();
		}

		switch (currentMesh)
		{
		case MeshType::PLANE:
			if (ImGui::SliderInt("stacks", &stacks, 2, 30))
			{
				isRecreate = true;
			}
			if (ImGui::SliderInt("slices", &slices, 2, 30))
			{
				isRecreate = true;
			}
			break;
		case MeshType::CUBE:
			if (ImGui::SliderInt("stacks", &stacks, 2, 10))
			{
				isRecreate = true;
			}
			if (ImGui::SliderInt("slices", &slices, 2, 10))
			{
				isRecreate = true;
			}
			break;
		case MeshType::SPHERE:
			if (ImGui::SliderInt("stacks", &stacks, 5, 35))
			{
				isRecreate = true;
			}
			if (ImGui::SliderInt("slices", &slices, 5, 35))
			{
				isRecreate = true;
			}
			break;
		}
	}

	//Object
	if (ImGui::CollapsingHeader("Object"))
	{
		//Select obj
		if (ImGui::BeginMenu("Select obj"))
		{
			if (ImGui::MenuItem("cube", "0"))
			{
				currentMesh = MeshType::OBJ;
				objPath = "../Game/assets/Models/cube.obj";
				isRecreate = true;
			}
			if (ImGui::MenuItem("car", "1"))
			{
				currentMesh = MeshType::OBJ;
				objPath = "../Game/assets/Models/car.obj";
				isRecreate = true;
			}
			if (ImGui::MenuItem("diamond", "2"))
			{
				currentMesh = MeshType::OBJ;
				objPath = "../Game/assets/Models/diamond.obj";
				isRecreate = true;
			}
			if (ImGui::MenuItem("dodecahedron", "3"))
			{
				currentMesh = MeshType::OBJ;
				objPath = "../Game/assets/Models/dodecahedron.obj";
				isRecreate = true;
			}
			if (ImGui::MenuItem("gourd", "4"))
			{
				currentMesh = MeshType::OBJ;
				objPath = "../Game/assets/Models/gourd.obj";
				isRecreate = true;
			}
			if (ImGui::MenuItem("sphere", "5"))
			{
				currentMesh = MeshType::OBJ;
				objPath = "../Game/assets/Models/sphere.obj";
				isRecreate = true;
			}
			if (ImGui::MenuItem("teapot", "6"))
			{
				currentMesh = MeshType::OBJ;
				objPath = "../Game/assets/Models/teapot.obj";
				isRecreate = true;
			}
			if (ImGui::MenuItem("vase", "7"))
			{
				currentMesh = MeshType::OBJ;
				objPath = "../Game/assets/Models/vase.obj";
				isRecreate = true;
			}
			if (ImGui::MenuItem("monkey", "8"))
			{
				currentMesh = MeshType::OBJ;
				objPath = "../Game/assets/Models/monkey.obj";
				isRecreate = true;
			}
			ImGui::EndMenu();
		}
	}

	//Material
	if (ImGui::CollapsingHeader("Material"))
	{
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

		ImGui::Checkbox("DrawNormals", &isDrawNormals);
	}

	//Lighting
	if (ImGui::CollapsingHeader("Lights"))
	{
		//Change Light Color
		ImGui::SliderFloat3("Light Color", &l.lightColor.x, 0.f, 1.f);

		//Change Ambient Strength
		ImGui::SliderFloat("Ambient Strength", &l.ambientStrength, 0.f, 1.f);

		//Change Specular Strength
		ImGui::SliderFloat("Specular Strength", &l.specularStrength, 0.f, 1.f);
	}

	ImGui::End();
}

void ProceduralMeshes::Restart()
{
	Engine::GetObjectManager().DestroyAllObjects();
}

void ProceduralMeshes::End()
{
	isFill = true;
	Engine::GetRenderManager()->SetPolygonType(PolygonType::FILL);
	Engine::GetRenderManager()->EnableLighting(false);

	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}

void ProceduralMeshes::RecreateMesh()
{
	if (currentMesh != MeshType::OBJ)
	{
		if (Engine::GetObjectManager().GetLastObject() != nullptr)
			Engine::GetObjectManager().Destroy(Engine::GetObjectManager().GetLastObject()->GetId());

		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(currentMesh, objPath, stacks, slices, { color[0], color[1], color[2], color[3] });
	}
	else
	{
		if (Engine::GetObjectManager().GetLastObject() != nullptr)
			Engine::GetObjectManager().Destroy(Engine::GetObjectManager().GetLastObject()->GetId());

		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, objPath, stacks, slices, { color[0], color[1], color[2], color[3] });
	}
}
