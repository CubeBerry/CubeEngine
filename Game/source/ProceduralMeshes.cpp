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

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, -1.f, 0.25f), glm::vec3{ 0.05f,0.05f,0.05f }, "1", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(0.f, 1.f, 0.f, 1.f));

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, -1.f, 0.5f), glm::vec3{ 0.05f,0.05f,0.05f }, "2", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(1.f, 0.f, 0.f, 1.f));

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, -1.f, 0.75f), glm::vec3{ 0.05f,0.05f,0.05f }, "3", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(0.f, 0.f, 1.f, 1.f));

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, -1.f, 0.1f), glm::vec3{ 0.05f,0.05f,0.05f }, "4", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(0.f, 0.f, 0.f, 1.f));

	currentMesh = MeshType::PLANE;
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "Mesh", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::PLANE, "", 1, 1, { 1.0, 0.0, 0.0, 1.0 });

	Engine::GetRenderManager()->LoadTexture("../Game/assets/monkey.png", "monkey", false);

	Engine::GetRenderManager()->LoadSkybox("../Game/assets/Skybox/HDR/Equirectangular/autumn_field_puresky_4k.hdr");
}

void ProceduralMeshes::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);

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
}

void ProceduralMeshes::ImGuiDraw(float /*dt*/)
{
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
	color = { 1.f, 0.f, 0.f, 1.f };
	angle[0] = 0;
	angle[1] = 0;

	Engine::GetRenderManager()->SetPolygonType(PolygonType::FILL);
	Engine::GetRenderManager()->DeletePointLights();
	Engine::GetRenderManager()->DeleteDirectionalLights();
	Engine::GetRenderManager()->DeleteSkybox();

	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().End();
}
