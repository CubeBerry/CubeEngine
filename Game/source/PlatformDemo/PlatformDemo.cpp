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
						/*if (target.second->GetPosition().y > object.second->GetPosition().y &&
							target.second->GetPosition().x - target.second->GetSize().x / 2.f <= object.second->GetPosition().x
							+ object.second->GetSize().x / 2.f && target.second->GetPosition().x + 
							target.second->GetSize().x / 2.f > object.second->GetPosition().x - object.second->GetSize().x / 2.f)
						{
						}*/
					}
				}
			}
		}
	}
}

void PlatformDemo::Init()
{
	Engine::Instance().GetObjectManager()->AddObject<PPlayer>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Player");

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,-192.f,0.f }, glm::vec3{ 320.f, 32.f,0.f }, "Wall", ObjectType::WALL);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,1.f });

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().x / 2.f,  Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().y / 2.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(1.f);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 128.f,-64.f,0.f }, glm::vec3{ 96.f, 32.f,0.f }, "Wall", ObjectType::WALL);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,1.f });

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().x / 2.f,  Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().y / 2.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(1.f);
}

void PlatformDemo::Update(float dt)
{
	CollideObjects();
}

#ifdef _DEBUG
void PlatformDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
	Engine::GetSoundManager()->MusicPlayerForImGui();
}
#endif

void PlatformDemo::Restart()
{
	End();
	Init();
}

void PlatformDemo::End()
{
	//Engine::Instance().GetParticleManager()->Clear();
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
}

