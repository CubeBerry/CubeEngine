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

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "0", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::CUBE, 2, 2, { 1.0, 0.0, 0.0, 1.0 });

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,-2.5f }, glm::vec3{ 1.f,1.f,1.f }, "1", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::PLANE, 2, 2, { 1.0, 0.0, 1.0, 1.0 });

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,-5.f }, glm::vec3{ 1.f,1.f,1.f }, "2", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::CUBE, 2, 2, { 0.0, 1.0, 0.0, 1.0 });
}

void ProceduralMeshes::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);

	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::U))
	{
		Engine::GetObjectManager().FindObjectWithName("0")->SetXRotate(
			Engine::GetObjectManager().FindObjectWithName("0")->GetRotate3D().x + 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::J))
	{
		Engine::GetObjectManager().FindObjectWithName("0")->SetXRotate(
			Engine::GetObjectManager().FindObjectWithName("0")->GetRotate3D().x - 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::I))
	{
		Engine::GetObjectManager().FindObjectWithName("0")->SetYRotate(
			Engine::GetObjectManager().FindObjectWithName("0")->GetRotate3D().y + 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::K))
	{
		Engine::GetObjectManager().FindObjectWithName("0")->SetYRotate(
			Engine::GetObjectManager().FindObjectWithName("0")->GetRotate3D().y - 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::O))
	{
		Engine::GetObjectManager().FindObjectWithName("0")->SetZRotate(
			Engine::GetObjectManager().FindObjectWithName("0")->GetRotate3D().z + 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::L))
	{
		Engine::GetObjectManager().FindObjectWithName("0")->SetZRotate(
			Engine::GetObjectManager().FindObjectWithName("0")->GetRotate3D().z - 10.f * dt);
	}

	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::NUMBER_1))
	{
		Engine::GetObjectManager().Destroy(Engine::GetObjectManager().FindObjectWithName("1")->GetId());
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::NUMBER_2))
	{
		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,-7.f }, glm::vec3{ 1.f,1.f,1.f }, "3", ObjectType::NONE);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::CUBE, 2, 2, { 0.0, 0.0, 0.0, 1.0 });
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::R))
	{
		Engine::GetGameStateManager().SetGameState(State::RESTART);
	}
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
