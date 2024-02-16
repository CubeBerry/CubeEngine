//Author: DOYEONG LEE
//Project: CubeEngine
//File: PocketBallDemo.cpp
#include "PocketBallDemo/PocketBallDemo.hpp"
#include "PocketBallDemo/Ball.hpp"
#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"

#include <iostream>
#include <cmath>

void PocketBallDemo::CollideObjects()
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
					if (target.second->GetObjectType() == ObjectType::GOAL && object.second->GetObjectType() == ObjectType::BALL)
					{
						int time = (rand() % (3 + 1)) + 1;
						int amount = (rand() % (8 + 4)) + 4;
						int colorR = (rand() % (10 + 0)) + 0;
						int colorG = (rand() % (10 + 0)) + 0;
						int colorB = (rand() % (10 + 0)) + 0;
						int colorA = (rand() % (10 + 5)) + 5;
						float x = object.second->GetPosition().x;
						float y = object.second->GetPosition().y;
						float speedX = (float)(rand() % (15 - (-30) + 1) - 15);
						float speedY = (float)(rand() % (15 - (-30) + 1) - 15);

						{
							Engine::Instance().GetParticleManager()->AddRandomParticle({ x,y,0.f }, { 4.f,4.f,0.f }, { speedX,speedY,0.f }, 0.f, static_cast<float>(time), amount,
								{ static_cast<float>(colorR * 0.1f),static_cast<float>(colorG * 0.1f),static_cast<float>(colorB * 0.1f),static_cast<float>(colorA * 0.1f) });
						}
						Engine::Instance().GetObjectManager()->Destroy(object.second.get()->GetId());
						pocketBallSystem->SetBallNum(--ballAmount);
					}
				}
			}
		}
	}
}

void PocketBallDemo::Init()
{
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/White.png", "White");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/1.png", "1");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/2.png", "2");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/3.png", "3");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/4.png", "4");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/5.png", "5");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/6.png", "6");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/Table.png", "Table");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/PocketBall/Arrow.png", "Arrow");

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,-38.f,-0.1f }, glm::vec3{ 368.f*2 + 32.f, 510.f + 88.f,0.f }, "Table");
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("Table");
	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "White", BallType::WHITE);
	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -120.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "1", BallType::OTHER);
	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -152.f,16.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "2", BallType::OTHER);
	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -152.f,-16.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "3", BallType::OTHER);
	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -184.f,32.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "4", BallType::OTHER);
	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -184.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "5", BallType::OTHER);
	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -184.f,-32.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "6", BallType::OTHER);

	{
		glm::vec2 tempS{ 0.f,0.f };
		Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,192.f,0.f }, glm::vec3{ 640.f, 32.f,0.f }, "Wall", ObjectType::WALL);
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,0.f });

		tempS = { Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().x / 2.f,Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().y / 2.f };
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, tempS.y });
		//Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, -(tempS.y - 16.f) });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x - 32.f, -tempS.y });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x + 32.f, -tempS.y });
		//Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, -(tempS.y - 16.f) });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, tempS.y });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);


		Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,-192.f,0.f }, glm::vec3{ 640.f, 32.f,0.f }, "Wall", ObjectType::WALL);
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,0.f });
		Engine::Instance().GetObjectManager()->GetLastObject()->SetRotate(180.f);

		tempS = { Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().x / 2.f,Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().y / 2.f };
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, tempS.y });
		//Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, -(tempS.y - 16.f) });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x - 32.f, -tempS.y });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x + 32.f, -tempS.y });
		//Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, -(tempS.y - 16.f) });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, tempS.y });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);


		Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ -336.f,0.f,0.f }, glm::vec3{ 352.f, 32.f,0.f }, "Wall", ObjectType::WALL);
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,0.f });

		tempS = { Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().x / 2.f,Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().y / 2.f };
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, tempS.y });
		//Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, -(tempS.y - 16.f) });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x - 32.f, -tempS.y });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x + 32.f, -tempS.y });
		//Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, -(tempS.y - 16.f) });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, tempS.y });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);
		Engine::Instance().GetObjectManager()->GetLastObject()->SetRotate(90.f);

		Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 336.f,0.f,0.f }, glm::vec3{ 352.f, 32.f,0.f }, "Wall", ObjectType::WALL);
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,0.f });

		tempS = { Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().x / 2.f,Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().y / 2.f };
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, tempS.y });
		//Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, -(tempS.y - 16.f) });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x - 32.f, -tempS.y });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x + 32.f, -tempS.y });
		//Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, -(tempS.y - 16.f) });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, tempS.y });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);
		Engine::Instance().GetObjectManager()->GetLastObject()->SetRotate(270.f);

		Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 336.f,192.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Goal", ObjectType::GOAL);
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 0.f,0.f,0.f,0.5f });

		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 2.f, 2.f });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);

		Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ -336.f,192.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Goal", ObjectType::GOAL);
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 0.f,0.f,0.f,0.5f });

		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 2.f, 2.f });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);

		Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ -336.f,-192.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Goal", ObjectType::GOAL);
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 0.f,0.f,0.f,0.5f });

		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 2.f, 2.f });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);

		Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 336.f,-192.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Goal", ObjectType::GOAL);
		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 0.f,0.f,0.f,0.5f });

		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 2.f, 2.f });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);
	}

	ballAmount = 7;
	pocketBallSystem = new PocketBallSystem();
	pocketBallSystem->Init();
	pocketBallSystem->SetBallNum(ballAmount);
}

void PocketBallDemo::Update(float dt)
{
	CollideObjects();
	pocketBallSystem->Update(dt);
}

#ifdef _DEBUG
void PocketBallDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
	Engine::GetGameStateManager()->StateChanger();
	Engine::GetSoundManager()->MusicPlayerForImGui();
}
#endif

void PocketBallDemo::Restart()
{
	End();
	Init();
}

void PocketBallDemo::End()
{
	pocketBallSystem->End();
	delete pocketBallSystem;
	pocketBallSystem = nullptr;

	Engine::GetCameraManager()->Reset();
	Engine::GetParticleManager()->Clear();
	Engine::GetObjectManager()->DestroyAllObjects();
}

