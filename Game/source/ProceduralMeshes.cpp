//Author: JEYOON YU
//Project: CubeEngine
//File: ProceduralMeshes.cpp
#include "ProceduralMeshes.hpp"
#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

void ProceduralMeshes::Init()
{
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 45.f);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,4.f,-9.f }, glm::vec3{ 16.f,9.f,0.f }, "0", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::CUBE, 2, 2, { 1.0, 0.0, 0.0, 1.0 });
}

void ProceduralMeshes::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);
}

#ifdef _DEBUG
void ProceduralMeshes::ImGuiDraw(float /*dt*/)
{
	Engine::GetGameStateManager().StateChanger();
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
