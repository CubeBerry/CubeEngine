//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: ProceduralMeshes.cpp
#include "ProceduralMeshes.hpp"
#include "Engine.hpp"
#include "BasicComponents/Light.hpp"

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

	//Debug Lighting
	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, 0.f, 0.f), glm::vec3{ 0.05f,0.05f,0.05f }, "PointLight", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(0.f, 0.f, 1.f, 1.f));

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, 0.f, 0.f), glm::vec3{ 0.05f,0.05f,0.05f }, "PointLight2", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(0.f, 1.f, 0.f, 1.f));

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, -1.f, 0.f), glm::vec3{ 0.05f,0.05f,0.05f }, "DirectionalLight", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::DIRECTIONAL);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(1.f, 1.f, 1.f, 1.f));

	currentMesh = MeshType::PLANE;
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::PLANE, "", 1, 1, { 1.0, 0.0, 0.0, 1.0 });

	Engine::GetRenderManager()->LoadTexture("../Game/assets/monkey.png", "monkey", false);

	//Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 1.f,1.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mo", ObjectType::NONE);
	//Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	//Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/monkey.obj", 1, 1, { 1.0, 0.0, 0.0, 1.0 });
	//Engine::GetObjectManager().FindObjectWithName("Mo")->GetComponent<Sprite>()->ChangeTexture("monkey");

	Engine::GetRenderManager()->LoadSkyBox(
		"../Game/assets/Skybox/right.jpg",
		"../Game/assets/Skybox/left.jpg",
		"../Game/assets/Skybox/top.jpg",
		"../Game/assets/Skybox/bottom.jpg",
		"../Game/assets/Skybox/back.jpg",
		"../Game/assets/Skybox/front.jpg"
	);
}

void ProceduralMeshes::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);

	if (isRecreate)
	{
		RecreateMesh();
		isRecreate = false;
	}

#ifdef _DEBUG
	Engine::GetRenderManager()->DrawNormals(isDrawNormals);
#endif
	//Update Lighting Variables
	angle[0] += 50.f * dt;
	if (angle[0] >= 360.f) angle[0] -= 360.f;
	float radians1 = glm::radians(angle[0]);

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), radians1, glm::vec3(0.f, 1.f, 0.f));
	glm::vec4 rotatedPosition = rotationMatrix * glm::vec4(0.f, 0.f, 1.f, 1.f);
	Engine::GetObjectManager().FindObjectWithName("PointLight")->SetXPosition(rotatedPosition.x);
	Engine::GetObjectManager().FindObjectWithName("PointLight")->SetZPosition(rotatedPosition.z);

	//Update Lighting Variables 2
	angle[1] += 25.f * dt;
	if (angle[1] >= 360.f) angle[1] -= 360.f;
	float radians2 = glm::radians(-angle[1]);

	rotationMatrix = glm::rotate(glm::mat4(1), radians2, glm::vec3(1.f, 1.f, 0.f));
	rotatedPosition = rotationMatrix * glm::vec4(0.f, 0.f, 1.f, 1.f);
	Engine::GetObjectManager().FindObjectWithName("PointLight2")->SetXPosition(rotatedPosition.x);
	Engine::GetObjectManager().FindObjectWithName("PointLight2")->SetYPosition(rotatedPosition.y);
	Engine::GetObjectManager().FindObjectWithName("PointLight2")->SetZPosition(rotatedPosition.z);

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
			if (ImGui::MenuItem("Torus", "3"))
			{
				stacks = 15;
				slices = 15;
				currentMesh = MeshType::TORUS;
				isRecreate = true;
			}
			if (ImGui::MenuItem("Cylinder", "4"))
			{
				stacks = 10;
				slices = 10;
				currentMesh = MeshType::CYLINDER;
				isRecreate = true;
			}
			if (ImGui::MenuItem("Cone", "5"))
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
			if (ImGui::SliderInt("stacks", &stacks, 1, 30))
			{
				isRecreate = true;
			}
			if (ImGui::SliderInt("slices", &slices, 1, 30))
			{
				isRecreate = true;
			}
			break;
		case MeshType::CUBE:
			if (ImGui::SliderInt("stacks", &stacks, 1, 10))
			{
				isRecreate = true;
			}
			if (ImGui::SliderInt("slices", &slices, 1, 10))
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
		case MeshType::TORUS:
			if (ImGui::SliderInt("stacks", &stacks, 10, 35))
			{
				isRecreate = true;
			}
			if (ImGui::SliderInt("slices", &slices, 10, 35))
			{
				isRecreate = true;
			}
			break;
		case MeshType::CYLINDER:
			if (ImGui::SliderInt("stacks", &stacks, 5, 35))
			{
				isRecreate = true;
			}
			if (ImGui::SliderInt("slices", &slices, 5, 35))
			{
				isRecreate = true;
			}
			break;
		case MeshType::CONE:
			if (ImGui::SliderInt("stacks", &stacks, 3, 35))
			{
				isRecreate = true;
			}
			if (ImGui::SliderInt("slices", &slices, 3, 35))
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

		ImGui::ColorPicker3("Mesh Color", color.data());
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->SetColor(glm::vec4(color[0], color[1], color[2], color[3]));

		ImGui::Checkbox("DrawNormals", &isDrawNormals);
	}

	//Lighting
	/*static int selectedLightIndex = 0;
	if (ImGui::CollapsingHeader("Lights"))
	{
		if (ImGui::BeginMenu("Select Light"))
		{
			auto& lights = Engine::GetRenderManager()->GetPointLightUniforms();
			for (size_t i = 0; i < lights.size(); ++i)
			{
				if (ImGui::Selectable(("Light " + std::to_string(i)).c_str(), selectedLightIndex == i))
				{
					selectedLightIndex = static_cast<int>(i);
				}
			}
			ImGui::EndMenu();
		}

		if (selectedLightIndex >= 0 && selectedLightIndex < Engine::GetRenderManager()->GetPointLightUniforms().size())
		{
			auto& light = Engine::GetRenderManager()->GetPointLightUniforms()[selectedLightIndex];

			float color_[3] = { light.lightColor.x, light.lightColor.y, light.lightColor.z };
			if (ImGui::ColorEdit3("Light Color", color_))
			{
				light.lightColor.x = color_[0];
				light.lightColor.y = color_[1];
				light.lightColor.z = color_[2];
			}

			ImGui::SliderFloat("Ambient Strength", &light.ambientStrength, 0.f, 1.f);

			ImGui::SliderFloat("Specular Strength", &light.specularStrength, 0.f, 1.f);
		}
	}*/

	ImGui::End();

	Engine::GetCameraManager().CameraControllerImGui();
	Engine::GetObjectManager().ObjectControllerForImGui();
}

void ProceduralMeshes::Restart()
{
	End();
}

void ProceduralMeshes::End()
{
	stacks = 1;
	slices = 1;
	isFill = true;
	color = { 1.f, 0.f, 0.f, 1.f };
	isRecreate = false;
	isDrawNormals = false;
	angle[0] = 0;
	angle[1] = 0;

	Engine::GetRenderManager()->SetPolygonType(PolygonType::FILL);
	Engine::GetRenderManager()->DeletePointLights();
	Engine::GetRenderManager()->DeleteDirectionalLights();
	Engine::GetRenderManager()->DeleteSkyBox();

	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().End();
}

void ProceduralMeshes::RecreateMesh()
{
	if (currentMesh != MeshType::OBJ)
	{
		if (Engine::GetObjectManager().FindObjectWithName("Mesh") != nullptr)
			Engine::GetObjectManager().Destroy(Engine::GetObjectManager().FindObjectWithName("Mesh")->GetId());

		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(currentMesh, objPath, stacks, slices, { color[0], color[1], color[2], color[3] });
	}
	else
	{
		if (Engine::GetObjectManager().FindObjectWithName("Mesh") != nullptr)
			Engine::GetObjectManager().Destroy(Engine::GetObjectManager().FindObjectWithName("Mesh")->GetId());

		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, objPath, stacks, slices, { color[0], color[1], color[2], color[3] });
		if (objPath == "../Game/assets/Models/monkey.obj")
			Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->ChangeTexture("monkey");

		//if (objPath == "../Game/assets/Models/monkey.obj")
		//	Engine::GetObjectManager().FindObjectWithName("Mesh")->GetComponent<Sprite>()->ChangeTexture("monkey");
	}
}
