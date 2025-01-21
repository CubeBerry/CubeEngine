//Author: JEYOON YU
//Project: CubeEngine
//File: PBR.cpp
#include "PBR.hpp"
#include "Engine.hpp"
#include "BasicComponents/Light.hpp"

#include <glm/gtc/matrix_transform.hpp>

void PBR::Init()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::ThreeDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 1.f);
	Engine::GetCameraManager().SetFar(cFar);
	Engine::GetCameraManager().SetBaseFov(cFov);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,0.f,13.f });
	//Engine::GetCameraManager().SetCameraPosition(glm::vec3{ 2.f, 2.f, 2.f });
	//Engine::GetCameraManager().SetCenter(glm::vec3{ 0.f, 0.f, 0.f });

	//Debug Lighting
	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, 0.f, 0.f), glm::vec3{ 0.05f,0.05f,0.05f }, "PointLight", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(0.f, 0.f, 3.f, 1.f));

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, 0.f, 0.f), glm::vec3{ 0.05f,0.05f,0.05f }, "PointLight2", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(0.f, 1.f, 0.f, 1.f));

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, 0.f, -1.f), glm::vec3{ 0.05f,0.05f,0.05f }, "DirectionalLight", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::DIRECTIONAL);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(1.f, 1.f, 1.f, 1.f));

	for (int r = 0; r < 6; ++r)
	{
		for (int c = 0; c < 6; ++c)
		{
			float x = (r - 2.5f) * 1.2f;
			float y = (c - 2.5f) * 1.2f;

			float metallic = static_cast<float>(c) * 0.2f;
			float roughness = static_cast<float>(r) * 0.2f;

			Engine::GetObjectManager().AddObject<Object>(glm::vec3{ x, y, 0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
			Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
			Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/sphere.obj", 1, 1, { 1.0, 0.0, 0.0, 1.0 }, metallic, roughness);
		}
	}

	//Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f, 0.f, 0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
	//Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	//Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/sphere.obj", 1, 1, { 1.0, 0.0, 0.0, 1.0 }, 0.f, 0.01f);

	Engine::GetRenderManager()->LoadEquirectangularToSkyBox(true, "../Game/assets/Skybox/HDR/Equirectangular/billiard_hall_4k.hdr");
}

void PBR::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);

#ifdef _DEBUG
	Engine::GetRenderManager()->DrawNormals(isDrawNormals);
#endif
}

void PBR::ImGuiDraw(float /*dt*/)
{
	Engine::GetObjectManager().ObjectControllerForImGui();
}

void PBR::Restart()
{
	End();
}

void PBR::End()
{
	isDrawNormals = false;

	Engine::GetRenderManager()->SetPolygonType(PolygonType::FILL);
	//Engine::GetRenderManager()->DeletePointLights();
	//Engine::GetRenderManager()->DeleteDirectionalLights();
	Engine::GetRenderManager()->DeleteSkyBox();

	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().End();
}
