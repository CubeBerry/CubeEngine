//Author: DOYEONG LEE
//Project: CubeEngine
//File: 3DPhysicsDemo.cpp
#include "3DPhysicsDemo.hpp"
#include "BasicComponents/Physics3D.hpp"
#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

void PhysicsDemo::Init()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::ThreeDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 1.f);
	Engine::GetCameraManager().SetNear(0.5f);
	Engine::GetCameraManager().SetFar(45.f);
	Engine::GetCameraManager().SetBaseFov(22.5f);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,2.f,13.f });
	Engine::GetCameraManager().SetTarget(glm::vec3{ 0.f, 0.f,0.f });

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,-2.f,0.f }, glm::vec3{ 20.f,20.f,1.f }, "PLANE", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::PLANE, "", 2, 2, { 0.0, 0.8f, 0.0, 1.0 });
	Engine::GetObjectManager().GetLastObject()->AddComponent<Physics3D>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->SetBodyType(BodyType3D::BLOCK);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->AddCollidePolyhedronAABB({ 20.f,20.f,0.001f });

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "CUBE", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 });
	Engine::GetObjectManager().GetLastObject()->AddComponent<Physics3D>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->AddCollidePolyhedronAABB({ 1.f,1.f,1.f });
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->SetGravity(2.f);

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 1.0f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "CUBE1", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 });
	Engine::GetObjectManager().GetLastObject()->AddComponent<Physics3D>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->AddCollidePolyhedronAABB({ 1.f,1.f,1.f });
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->SetGravity(2.f);

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 1.0f,0.f,1.f }, glm::vec3{ 1.f,1.f,1.f }, "CUBE2", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 });
	Engine::GetObjectManager().GetLastObject()->AddComponent<Physics3D>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->AddCollidePolyhedronAABB({ 1.f,1.f,1.f });
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->SetGravity(2.f);

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -1.0f,0.f,-1.f }, glm::vec3{ 0.5f,0.5f,0.5f }, "CUBE3", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 });
	Engine::GetObjectManager().GetLastObject()->AddComponent<Physics3D>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->AddCollidePolyhedronAABB({ 0.5f,0.5f,0.5f });
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->SetGravity(2.f);

	l.lightPosition = glm::vec4(0.f, 20.f, 0.f, 1.f);
	l.lightColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
	//viewPosition == cameraPosition
	l.viewPosition = glm::vec4(Engine::GetCameraManager().GetCameraPosition(), 1.f);
	l.ambientStrength = 0.1f;
	l.specularStrength = 0.5f;
	l.isLighting = true;
	Engine::GetRenderManager()->EnableLighting(true);
}

void PhysicsDemo::Update(float dt)
{
	//Engine::GetRenderManager()->SetPolygonType(PolygonType::LINE);

	for (auto& target : Engine::GetObjectManager().GetObjectMap())
	{
		for (auto& object : Engine::GetObjectManager().GetObjectMap())
		{
			if (target.second != nullptr && object.second != nullptr && target.second != object.second)
			{
				if (target.second->HasComponent<Physics3D>() == true && object.second->HasComponent<Physics3D>() == true)
				{
					if (target.second->GetComponent<Physics3D>()->CheckCollision(object.second.get()))
					{
						//std::cout << "!" << std::endl;
					}
				}
			}
		}
	}

	{
		glm::vec3 movement(0.0f);
		float speed = 20.0f * dt;

		if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::UP))
		{
			movement += Engine::GetCameraManager().GetBackVector() * speed;
		}
		if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::DOWN))
		{
			movement -= Engine::GetCameraManager().GetBackVector() * speed;
		}
		if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::RIGHT))
		{
			movement += Engine::GetCameraManager().GetRightVector() * speed;
		}
		if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::LEFT))
		{
			movement -= Engine::GetCameraManager().GetRightVector() * speed;
		}

		movement.y = 0;

		if (glm::length(movement) > 0)
		{
			movement = glm::normalize(movement) * speed;
			Engine::GetObjectManager().FindObjectWithName("CUBE")->GetComponent<Physics3D>()->AddForce(movement);
		}
	}

	//Update Lighting Variables
	l.viewPosition = glm::vec4(Engine::GetCameraManager().GetCameraPosition(), 1.f);
	Engine::GetRenderManager()->UpdateLighting(l.lightPosition, l.lightColor, l.viewPosition, l.ambientStrength, l.specularStrength);

	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::R))
	{
		Engine::GetGameStateManager().SetGameState(State::RESTART);
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::NUMBER_1))
	{
		if (Engine::GetObjectManager().FindObjectWithName("TEMP") == nullptr)
		{
			Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -1.0f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "TEMP", ObjectType::NONE);
			Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
			Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 1.0, 1.0, 1.0, 1.0 });
			Engine::GetObjectManager().GetLastObject()->AddComponent<Physics3D>();
			Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->AddCollidePolyhedronAABB({ 1.f,1.f,1.f });
			Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->SetGravity(2.f);
		}
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::NUMBER_2))
	{
		Engine::GetObjectManager().Destroy(Engine::GetObjectManager().FindObjectWithName("TEMP")->GetId());
	}

	Engine::GetCameraManager().ControlCamera(dt);
}

#ifdef _DEBUG
void PhysicsDemo::ImGuiDraw(float /*dt*/)
{
	Engine::GetGameStateManager().StateChanger();
	Engine::GetCameraManager().CameraControllerImGui();
	Engine::GetObjectManager().ObjectControllerForImGui();
}
#endif

void PhysicsDemo::Restart()
{
	Engine::GetObjectManager().DestroyAllObjects();
}

void PhysicsDemo::End()
{
	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}
