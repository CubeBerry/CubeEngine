//Author: DOYEONG LEE
//Project: CubeEngine
//File: PlatformDemo.cpp
#include "PlatformDemo/PlatformDemo.hpp"
#include "Engine.hpp"

#include "PlatformDemo/PPlayer.hpp"
#include "PlatformDemo/PEnemy.hpp"
#include "PlatformDemo/PBullet.hpp"
#include "PlatformDemo/PEnemyBullet.hpp"

#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

void PlatformDemo::CollideObjects()
{
	for (auto& target : Engine::GetObjectManager()->GetObjectMap())
	{
		for (auto& object : Engine::GetObjectManager()->GetObjectMap())
		{
			if (target.second != nullptr && object.second != nullptr && target.second != object.second
				&& target.second->HasComponent<Physics2D>() == true && object.second->HasComponent<Physics2D>() == true)
			{
				if ((target.second->GetObjectType() != ObjectType::WALL && object.second->GetObjectType() == ObjectType::WALL)
					|| (target.second->GetObjectType() == ObjectType::WALL && object.second->GetObjectType() != ObjectType::WALL))
				{
					target.second->GetComponent<Physics2D>()->CheckCollision(*object.second);
				}
				else if (target.second->GetObjectType() == ObjectType::BULLET && object.second->GetObjectType() == ObjectType::ENEMY)
				{
					if (static_cast<PEnemy*>(object.second.get())->GetInvincibleState() == false && target.second->GetComponent<Physics2D>()->CheckCollision(*object.second) == true)
					{
						PEnemy* e = static_cast<PEnemy*>(object.second.get());
						PBullet* b = static_cast<PBullet*>(target.second.get());

						e->SetHp(e->GetHp() - b->GetDamage());
						e->SetIsHit(true);

						Engine::GetObjectManager()->Destroy(b->GetId());
						b = nullptr;
						e = nullptr;
						break;
					}
				}
				else if (target.second->GetObjectType() == ObjectType::PLAYER && object.second->GetObjectType() == ObjectType::ENEMYBULLET)
				{
					if (static_cast<PPlayer*>(target.second.get())->GetInvincibleState() == false && target.second->GetComponent<Physics2D>()->CheckCollision(*object.second) == true)
					{
						PPlayer* p = static_cast<PPlayer*>(target.second.get());
						PEnemyBullet* b = static_cast<PEnemyBullet*>(object.second.get());

						platformDemoSystem->HpDecrease(b->GetDamage());
						p->SetInvincibleState(true);

						Engine::GetObjectManager()->Destroy(b->GetId());
						p = nullptr;
						b = nullptr;
						break;
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

	Engine::GetVKRenderManager()->LoadTexture("../Game/assets/PlatformDemo/train_editor.png", "train_editor");
	Engine::GetVKRenderManager()->LoadTexture("../Game/assets/PlatformDemo/building1.png", "building1");
	Engine::GetVKRenderManager()->LoadTexture("../Game/assets/PlatformDemo/building2.png", "building2");
	Engine::GetVKRenderManager()->LoadTexture("../Game/assets/PlatformDemo/building3.png", "building3");
	Engine::GetVKRenderManager()->LoadTexture("../Game/assets/PlatformDemo/rail.png", "rail");
	Engine::GetVKRenderManager()->LoadTexture("../Game/assets/PlatformDemo/TrainSide.png", "trainSide");

	//platformDemoSystem->SetIsEditorMod(true);

	platformDemoSystem->LoadLevelData("../Game/assets/PlatformDemo/Stage.txt");
	Engine::GetObjectManager()->AddObject<PPlayer>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 64.f, 96.f,0.f }, "Player");

	Engine::GetObjectManager()->AddObject<PEnemy>(glm::vec3{ 64.f,196.f,0.f }, glm::vec3{ 64.f, 128.f,0.f }, "Enemy", EnemyType::NORMAL);
	Engine::GetObjectManager()->AddObject<PEnemy>(glm::vec3{ -128.f,196.f,0.f }, glm::vec3{ 64.f, 96.f,0.f }, "Enemy", EnemyType::NORMAL);

	platformDemoSystem->InitHealthBar();
}

void PlatformDemo::Update(float dt)
{
	if (Engine::GetInputManager()->IsKeyPressedOnce(KEYBOARDKEYS::NUMBER_1))
	{
		Engine::GetGameStateManager()->ChangeLevel(GameLevel::POCKETBALL);
	}
	else if(Engine::GetInputManager()->IsKeyPressedOnce(KEYBOARDKEYS::NUMBER_2))
	{
		Engine::GetGameStateManager()->ChangeLevel(GameLevel::PLATFORMDEMO);
	}

	CollideObjects();
	platformDemoSystem->Update(dt);
}

#ifdef _DEBUG
void PlatformDemo::ImGuiDraw(float dt)
{
	ImGui::ShowDemoWindow();
	Engine::GetGameStateManager()->StateChanger();
	Engine::GetSoundManager()->MusicPlayerForImGui();
	platformDemoSystem->UpdateMapEditor(dt);
}
#endif

void PlatformDemo::Restart()
{
	End();
	Init();
}

void PlatformDemo::End()
{
	platformDemoSystem->End();
	delete platformDemoSystem;
	platformDemoSystem = nullptr;

	Engine::GetCameraManager()->Reset();
	Engine::GetParticleManager()->Clear();
	Engine::GetObjectManager()->DestroyAllObjects();
}

