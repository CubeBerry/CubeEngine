//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimationDemo.cpp
#include "SkeletalAnimationDemo.hpp"
#include "Engine.hpp"
#include "BasicComponents/Light.hpp"
#include "BasicComponents/DynamicSprite.hpp"
#include "BasicComponents/StaticSprite.hpp"
#include "BasicComponents/SkeletalAnimator.hpp"
#include "BasicComponents/SkeletalAnimationStateMachine.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <string>

void SkeletalAnimationDemo::Init()
{
	Engine::GetRenderManager()->SetRenderType(RenderType::ThreeDimension);
	Engine::GetCameraManager().Init(Engine::GetWindow().GetWindowSize(), CameraType::ThreeDimension, 1.f);
	Engine::GetCameraManager().SetNear(cNear);
	Engine::GetCameraManager().SetFar(cFar);
	Engine::GetCameraManager().SetBaseFov(cFov);
	Engine::GetCameraManager().SetCameraSensitivity(10.f);
	Engine::GetCameraManager().SetCameraPosition({ 0.f,2.f,10.f });

	// Player with Skeletal Animation
	Engine::GetObjectManager().AddObject<Object>(glm::vec3(0, 0, 0), glm::vec3(0.01f, 0.01f, 0.01f), "Player");
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(
		MeshType::OBJ,
		"../Game/assets/Models/AnimationModels/Idle.fbx",
		1, 1,
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
	);
	Engine::GetObjectManager().GetLastObject()->AddComponent<SkeletalAnimator>();
	Engine::GetObjectManager().GetLastObject()->AddComponent<SkeletalAnimationStateMachine>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->AddState("Idle", "../Game/assets/Models/AnimationModels/Idle.fbx");
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->AddState("Walking", "../Game/assets/Models/AnimationModels/Walking.fbx");
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->AddState("Punch", "../Game/assets/Models/AnimationModels/Quad Punch.fbx");
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->AddState("Swing Dancing", "../Game/assets/Models/AnimationModels/Swing Dancing.fbx");
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->AddState("Hip Hop Dancing", "../Game/assets/Models/AnimationModels/Hip Hop Dancing.fbx");
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->AddState("Thriller_1.fbx", "../Game/assets/Models/AnimationModels/Thriller_1.fbx");
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->AddState("Thriller_2.fbx", "../Game/assets/Models/AnimationModels/Thriller_2.fbx");
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->AddState("Thriller_3.fbx", "../Game/assets/Models/AnimationModels/Thriller_3.fbx");
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->AddState("Thriller_4.fbx", "../Game/assets/Models/AnimationModels/Thriller_4.fbx");
	Engine::GetObjectManager().GetLastObject()->GetComponent<SkeletalAnimationStateMachine>()->ChangeState("Idle");

	// Plane Stage
	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 10.f,0.001f,10.f }, "Stage", ObjectType::NONE);
	Engine::GetObjectManager().GetLastObject()->AddComponent<DynamicSprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::CUBE, "", 1, 1, { 0.5f, 0.5f, 0.5f, 1.0f });

	// 16 Point Lights
	for (int i = 0; i < 16; ++i)
	{
		std::string lightName = "DemoPointLight" + std::to_string(i);
		Engine::GetObjectManager().AddObject<Object>(glm::vec3(0.f, 0.f, 0.f), glm::vec3{ 0.05f,0.05f,0.05f }, lightName, ObjectType::NONE);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Light>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->AddLight(LightType::POINT, 1.f, 1.f);
		Engine::GetObjectManager().GetLastObject()->GetComponent<Light>()->SetColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
	}

	Engine::GetRenderManager()->LoadSkybox("../Game/assets/Skybox/HDR/park_music_stage_4k.hdr");
}

void SkeletalAnimationDemo::Update(float dt)
{
	Engine::GetCameraManager().ControlCamera(dt);

	time += dt;
	float radius = 3.f;

    // Update 16 lights position and colors
	for (int i = 0; i < 16; ++i)
	{
		std::string lightName = "DemoPointLight" + std::to_string(i);
		float angle = (360.f / 16.f) * i + time * 50.f; // Rotating slowly
		float radians = glm::radians(angle);
        
		float x = cos(radians) * radius;
		float z = sin(radians) * radius;

        // Color changing over time using sine waves for RGB
        float r = (sin(time * 2.f + i * 0.5f) + 1.f) * 0.5f;
        float g = (sin(time * 2.f + i * 0.5f + 2.f) + 1.f) * 0.5f;
        float b = (sin(time * 2.f + i * 0.5f + 4.f) + 1.f) * 0.5f;

		Object* lightObj = Engine::GetObjectManager().FindObjectWithName(lightName);
		if (lightObj)
		{
			lightObj->SetXPosition(x);
			lightObj->SetYPosition(0.5f);
			lightObj->SetZPosition(z);
			lightObj->GetComponent<Light>()->SetColor(glm::vec4(r, g, b, 1.f));
		}
	}
}

void SkeletalAnimationDemo::ImGuiDraw(float /*dt*/)
{
	Engine::GetCameraManager().CameraControllerImGui();
	Engine::GetObjectManager().ObjectControllerForImGui();
}

void SkeletalAnimationDemo::Restart()
{
	End();
}

void SkeletalAnimationDemo::End()
{
	time = 0;

	Engine::GetRenderManager()->SetPolygonType(PolygonType::FILL);
	Engine::GetRenderManager()->DeletePointLights();
	Engine::GetRenderManager()->DeleteDirectionalLights();
	Engine::GetRenderManager()->DeleteSkybox();

	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().End();
}
