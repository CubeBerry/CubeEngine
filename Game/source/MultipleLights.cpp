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
	Engine::GetCameraManager().SetCameraPosition({ 20.f,15.f,20.f });
	Engine::GetCameraManager().SetTarget(glm::vec3{ 0.f, 5.f,0.f });

	// Plane
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 20.f,20.f,1.f }, "PLANE", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::PLANE, "", 2, 2, { 1.f, 1.f, 1.f, 1.f }, 1.f, 0.11f);

	// Sphere
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,5.f,0.f }, glm::vec3{ 10.f,10.f,10.f }, "SPHERE", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/sphere.obj", 1, 1, { 1.f, 1.f, 1.f, 1.f }, 1.f, 0.15f);

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
			Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT, 1.f);
			float r = 0.2f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.8f));
			float g = 0.2f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.8f));
			float b = 0.2f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.8f));
			Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(r, g, b, 1.f));
		}
	}

	Engine::GetRenderManager()->LoadSkybox("../Game/assets/Skybox/HDR/lebombo_4k.hdr");
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
}
