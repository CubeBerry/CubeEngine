//Author: JEYOON YU
//Project: CubeEngine
//File: MultipleLights.cpp
#include "MultipleLights.hpp"
#include "BasicComponents/Physics3D.hpp"
#include "BasicComponents/Light.hpp"
#include "Engine.hpp"

void MultipleLights::Init()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::ThreeDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 1.f);
	Engine::GetCameraManager().SetNear(0.001f);
	Engine::GetCameraManager().SetFar(1000.f);
	Engine::GetCameraManager().SetBaseFov(22.5f);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,2.f,13.f });
	Engine::GetCameraManager().SetTarget(glm::vec3{ 0.f, 0.f,0.f });

	// Plane
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,-2.f,0.f }, glm::vec3{ 20.f,20.f,1.f }, "PLANE", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::PLANE, "", 2, 2, { 0.0, 0.8f, 0.0, 1.0 }, 0.f, 0.f);

	// Cube
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,-1.5f,3.f }, glm::vec3{ 1.f,1.f,1.f }, "CUBE", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 }, 0.5f, 0.5f);

	// Cube
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 2.0f,-1.5f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "CUBE1", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 }, 0.5f, 0.5f);

	// Cube
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 1.0f,-1.5f,1.f }, glm::vec3{ 1.f,1.f,1.f }, "CUBE2COn", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 }, 0.5f, 0.5f);

	// Cube
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -1.0f,-1.5f,-1.f }, glm::vec3{ 0.5f,0.5f,0.5f }, "CUBE3", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 }, 0.5f, 0.5f);

	// Sphere
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -1.0f,-1.5f,-2.f }, glm::vec3{ 0.5f,0.5f,0.5f }, "SPHERE", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/sphere.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 }, 0.5f, 0.5f);

	// Light
	// Total Light = 14 * 14 = 196
	int rowCount{ 14 };
	int columnCount{ 14 };
	float range{ 10.f };
	float startX{ -range / 2.f };
	float startZ{ -range / 2.f };
	float stepX{ range / static_cast<float>(rowCount > 1 ? rowCount - 1 : 1) };
	float stepZ{ range / static_cast<float>(columnCount > 1 ? columnCount - 1 : 1) };

	for (int r = 0; r < rowCount; ++r)
	{
		for (int c = 0; c < columnCount; ++c)
		{
			float x = startX + (static_cast<float>(r) * stepX);
			float z = startZ + (static_cast<float>(c) * stepZ);

			Engine::GetObjectManager().AddObject<Object>(glm::vec3(x, 0.5f, z), glm::vec3{ 0.05f,0.05f,0.05f }, "PointLight" + std::to_string(r + c), ObjectType::NONE);
			Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
			Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT, 20.f);
			float r = 0.2f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.8f));
			float g = 0.2f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.8f));
			float b = 0.2f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.8f));
			Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(r, g, b, 1.f));
		}
	}

	Engine::GetRenderManager()->LoadSkybox("../Game/assets/Skybox/HDR/snowy_forest_path_02_4k.hdr");
}

void MultipleLights::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);
}

void MultipleLights::ImGuiDraw(float /*dt*/)
{
	Engine::GetCameraManager().CameraControllerImGui();
	Engine::GetObjectManager().ObjectControllerForImGui();
}

void MultipleLights::Restart()
{
	End();
}

void MultipleLights::End()
{
	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().End();
	Engine::GetRenderManager()->DeletePointLights();
	Engine::GetRenderManager()->DeleteDirectionalLights();
	Engine::GetRenderManager()->DeleteSkybox();
}
