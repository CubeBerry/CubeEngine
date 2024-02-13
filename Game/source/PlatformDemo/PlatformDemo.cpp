//Author: DOYEONG LEE
//Project: CubeEngine
//File: PlatformDemo.cpp
#include "PlatformDemo/PlatformDemo.hpp"
#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "PlatformDemo/PPlayer.hpp"
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"

#include <iostream>
#include <cmath>

void PlatformDemo::CollideObjects()
{
	for (auto& target : Engine::Instance().GetObjectManager()->GetObjectMap())
	{
		for (auto& object : Engine::Instance().GetObjectManager()->GetObjectMap())
		{
			if (target.second != nullptr && object.second != nullptr && target.second != object.second
				&& target.second->HasComponent<Physics2D>() == true && object.second->HasComponent<Physics2D>() == true)
			{
				if (target.second->GetComponent<Physics2D>()->CheckCollision(*object.second) == true)
				{
					if (target.second->GetObjectType() == ObjectType::PLAYER && object.second->GetObjectType() == ObjectType::WALL)
					{
					}
				}
			}
		}
	}
}

void PlatformDemo::Init()
{
	platformDemoSystem = new PlatformDemoSystem();
	platformDemoSystem->Init();

	Engine::Instance().GetObjectManager()->AddObject<PPlayer>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Player");
	platformDemoSystem->LoadLevelData("../Game/assets/PlatformDemo/Stage.txt");
	//platformDemoSystem->SetIsEditorMod(true);
}

void PlatformDemo::Update(float dt)
{
	CollideObjects();
}

#ifdef _DEBUG
void PlatformDemo::ImGuiDraw(float dt)
{
	ImGui::ShowDemoWindow();
	Engine::GetGameStateManager()->StateChanger();
	Engine::GetSoundManager()->MusicPlayerForImGui();
	platformDemoSystem->Update(dt);
}
#endif

void PlatformDemo::Restart()
{
	End();
	Init();
}

void PlatformDemo::End()
{
	delete platformDemoSystem;
	platformDemoSystem = nullptr;

	Engine::Instance().GetParticleManager()->Clear();
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
}

