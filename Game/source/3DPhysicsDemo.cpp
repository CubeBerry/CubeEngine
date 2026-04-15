//Author: DOYEONG LEE
//Project: CubeEngine
//File: 3DPhysicsDemo.cpp
#include "3DPhysicsDemo.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "BasicComponents/Physics3D.hpp"
#include "BasicComponents/Light.hpp"
#include "Engine.hpp"

#include <random>
#include <glm/gtc/matrix_transform.hpp>

void PhysicsDemo::Init()
{
	if (mode == PhysicsMode::ThreeDimension)
	{
		Init3D();
	}
	else
	{
		Init2D();
	}
}

void PhysicsDemo::Init3D()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::ThreeDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 1.f);
	Engine::GetCameraManager().SetNear(0.001f);
	Engine::GetCameraManager().SetFar(1000.f);
	Engine::GetCameraManager().SetBaseFov(22.5f);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,2.f,13.f });
	Engine::GetCameraManager().SetTarget(glm::vec3{ 0.f, 0.f,0.f });

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,-2.f,0.f }, glm::vec3{ 20.f,20.f,1.f }, "PLANE", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->SetXRotate(90.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::PLANE, "", 2, 2, { 0.0, 0.8f, 0.0, 1.0 }, 0.f, 0.f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Physics3D>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->SetBodyType(BodyType3D::BLOCK);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->AddCollidePolyhedronAABB({ 20.f,20.f,1.f });

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,3.f }, glm::vec3{ 1.f,1.f,1.f }, "CUBE", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 0.0, 1.0, 1.0 }, 0.5f, 0.5f);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Physics3D>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->AddCollidePolyhedronAABB({ 1.f,1.f,1.f });
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->SetGravity(2.f);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Physics3D>()->SetCollisionDetectionMode(CollisionDetectionMode::CONTINUOUS);

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, 0.5f, 0.f), glm::vec3{ 0.1f,0.1f,0.1f }, "LIGHT", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT, 25.f, 100.f);
	Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(1.f, 1.f, 1.f, 1.f));

	Engine::GetRenderManager()->LoadSkybox("../Game/assets/Skybox/HDR/snowy_forest_path_02_4k.hdr");
}

void PhysicsDemo::Init2D()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::TwoDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::TwoDimension, 1.f);

	// Create 2D Floor
	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, -300.f, 0.f), glm::vec3(1280.f, 50.f, 1.f), "Floor2D", ObjectType::NONE);
	Object* floor = Engine::GetObjectManager().GetLastObject();
	floor->AddComponent<DynamicSprite>();
	floor->GetComponent<DynamicSprite>()->AddQuad({ 0.5f, 0.5f, 0.5f, 1.0f });
	floor->AddComponent<Physics2D>();
	floor->GetComponent<Physics2D>()->AddCollidePolygonAABB(glm::vec2(1280.f, 50.f));
	floor->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
}

void PhysicsDemo::ClearScene()
{
	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().End();
	if (mode == PhysicsMode::ThreeDimension)
	{
		Engine::GetRenderManager()->DeletePointLights();
		Engine::GetRenderManager()->DeleteDirectionalLights();
		Engine::GetRenderManager()->DeleteSkybox();
	}
}

void PhysicsDemo::Update(float dt)
{
	{
		if (mode == PhysicsMode::ThreeDimension)
		{
			glm::vec3 movement(0.0f);
			float speed = 200.0f * dt;

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


			for (auto& target : Engine::GetObjectManager().GetObjectMap())
			{
				for (auto& object : Engine::GetObjectManager().GetObjectMap())
				{
					if (target.second != nullptr && object.second != nullptr && target.second != object.second)
					{
						if (target.second->HasComponent<Physics3D>() == true && object.second->HasComponent<Physics3D>() == true)
						{
							target.second->GetComponent<Physics3D>()->CheckCollision(object.second.get());
						}
					}
				}
			}

			movement.y = 0;

			if (glm::length(movement) > 0)
			{
				movement = glm::normalize(movement) * speed;
				Object* cube = Engine::GetObjectManager().FindObjectWithName("CUBE");
				if (cube != nullptr && cube->HasComponent<Physics3D>())
				{
					cube->GetComponent<Physics3D>()->AddForce(movement);
				}
			}
		}
		else // TwoDimension
		{
			for (auto& target : Engine::GetObjectManager().GetObjectMap())
			{
				for (auto& object : Engine::GetObjectManager().GetObjectMap())
				{
					if (target.second != nullptr && object.second != nullptr && target.second != object.second)
					{
						if (target.second->HasComponent<Physics2D>() == true && object.second->HasComponent<Physics2D>() == true)
						{
							target.second->GetComponent<Physics2D>()->CheckCollision(object.second.get());
						}
					}
				}
			}
		}
	}

	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::R))
	{
		Engine::GetGameStateManager().SetGameState(State::RESTART);
	}

	Engine::GetCameraManager().ControlCamera(dt);
}

void PhysicsDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::Begin("Physics Control");
	/*if (mode == PhysicsMode::ThreeDimension)
	{
		if (ImGui::Button("Switch to 2D"))
		{
			ClearScene();
			mode = PhysicsMode::TwoDimension;
			Init();
		}
	}
	else
	{
		if (ImGui::Button("Switch to 3D"))
		{
			ClearScene();
			mode = PhysicsMode::ThreeDimension;
			Init();
		}
	}*/

	if (ImGui::Button("Spawn Object"))
	{
		if (mode == PhysicsMode::ThreeDimension)
		{
			Spawn3D();
		}
		else
		{
			Spawn2D();
		}
	}
	ImGui::End();

	Engine::GetCameraManager().CameraControllerImGui();
	Engine::GetObjectManager().ObjectControllerForImGui();
}

void PhysicsDemo::Restart()
{
	End();
}

void PhysicsDemo::End()
{
	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().End();
	if (mode == PhysicsMode::ThreeDimension)
	{
		Engine::GetRenderManager()->DeletePointLights();
		Engine::GetRenderManager()->DeleteDirectionalLights();
		Engine::GetRenderManager()->DeleteSkybox();
	}
}

void PhysicsDemo::Spawn3D()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> disType(0, 1);
	std::uniform_real_distribution<float> disPosXZ(-4.0f, 4.0f);
	std::uniform_real_distribution<float> disPosY(4.0f, 8.0f);
	std::uniform_real_distribution<float> disScale(0.4f, 1.2f);
	std::uniform_real_distribution<float> disColor(0.2f, 1.0f);
	std::uniform_real_distribution<float> disRot(-180.0f, 180.0f);

	int type = disType(gen);
	float s = disScale(gen);
	glm::vec3 randomPos = { disPosXZ(gen), disPosY(gen), disPosXZ(gen) };
	glm::vec3 randomScale = { s, s, s };
	glm::vec4 randomColor = { disColor(gen), disColor(gen), disColor(gen), 1.0f };

	Engine::GetObjectManager().AddObject<Object>(randomPos, randomScale, "PhysicsObj", ObjectType::NONE);
	Object* newObj = Engine::GetObjectManager().GetLastObject();
	newObj->SetRotate({ disRot(gen), disRot(gen), disRot(gen) });

	newObj->AddComponent<DynamicSprite>();
	newObj->AddComponent<Physics3D>();
	Physics3D* phys = newObj->GetComponent<Physics3D>();

	if (type == 0)
	{
		newObj->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, randomColor, 0.5f, 0.5f);
		phys->AddCollidePolyhedronAABB(randomScale);
		phys->SetEnableRotationalPhysics(true);
	}
	else
	{
		newObj->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/sphere.obj", 1, 1, randomColor, 0.5f, 0.5f);
		phys->AddCollideSphere(s);
		phys->SetRestitution(1.f);
	}

	phys->SetGravity(9.8f);
	phys->SetMass(s * s * s);
}

void PhysicsDemo::Spawn2D()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	//std::uniform_real_distribution<float> disPosX(-400.0f, 400.0f);
	//std::uniform_real_distribution<float> disPosY(200.0f, 400.0f);
	std::uniform_real_distribution<float> disScale(30.f, 60.f);
	std::uniform_real_distribution<float> disColor(0.2f, 1.0f);
	std::uniform_real_distribution<float> disRot(0.0f, 360.0f);

	float s = disScale(gen);
	//glm::vec3 randomPos = { disPosX(gen), disPosY(gen), 0.f };
	glm::vec3 randomScale = { s, s, 1.f };
	glm::vec4 randomColor = { disColor(gen), disColor(gen), disColor(gen), 1.0f };

	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, 0.f, 0.f), randomScale, "PhysicsObj2D", ObjectType::NONE);
	Object* newObj = Engine::GetObjectManager().GetLastObject();
	newObj->SetRotate(disRot(gen));

	newObj->AddComponent<DynamicSprite>();
	newObj->AddComponent<Physics2D>();
	Physics2D* phys = newObj->GetComponent<Physics2D>();

	newObj->GetComponent<DynamicSprite>()->AddQuad(randomColor);
	phys->AddCollidePolygonAABB(glm::vec2(s, s));
	phys->SetMaxVelocity(glm::vec2(1000.f, 1000.f));
	phys->SetEnableRotationalPhysics(true);

	phys->SetGravity(980.0f);
	phys->SetMass(100.f);
}
