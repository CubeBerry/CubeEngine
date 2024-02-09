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
	for (auto& target : Engine::GetObjectManager()->GetObjectMap())
	{
		for (auto& object : Engine::GetObjectManager()->GetObjectMap())
		{
			if (target.second != nullptr && object.second != nullptr && target.second != object.second
				&& target.second->HasComponent<Physics2D>() == true && object.second->HasComponent<Physics2D>() == true)
			{
				if (target.second->GetComponent<Physics2D>()->CheckCollision(*object.second) == true)
				{
					if (target.second->GetObjectType() == ObjectType::WALL && object.second->GetObjectType() == ObjectType::BALL)
					{
						//Physics2D* BallP = object.second->GetComponent<Physics2D>();

						//glm::vec2 ballCenter = glm::vec2(object.second->GetPosition().x, object.second->GetPosition().y) + glm::vec2(BallP->GetCircleCollideRadius(), BallP->GetCircleCollideRadius());
						//glm::vec2 wallCenter = glm::vec2(target.second->GetPosition().x, target.second->GetPosition().y) + 0.5f * glm::vec2(target.second->GetSize().x, target.second->GetSize().y);

						//glm::vec2 collisionVector = ballCenter - wallCenter;
						//float collisionAngle = atan2(collisionVector.y, collisionVector.x);

						//float newVelocityMagnitude = std::hypot(BallP->GetVelocity().x, BallP->GetVelocity().y);

						//// 충돌 위치에 따라 반발력을 적용
						//BallP->SetVelocity({ newVelocityMagnitude * cos(collisionAngle), newVelocityMagnitude * sin(collisionAngle) });
					}
				}
			}
		}
	}
}

void PocketBallDemo::Init()
{
	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "A", ObjectType::BALL);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(0);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollideCircle(16.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);

	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ - 120.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "1", ObjectType::BALL);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollideCircle(16.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);

	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -152.f,16.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "2", ObjectType::BALL);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollideCircle(16.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);

	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -152.f,-16.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "3", ObjectType::BALL);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollideCircle(16.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);

	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -184.f,32.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "4", ObjectType::BALL);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollideCircle(16.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);

	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -184.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "5", ObjectType::BALL);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollideCircle(16.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);

	Engine::Instance().GetObjectManager()->AddObject<Ball>(glm::vec3{ -184.f,-32.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "6", ObjectType::BALL);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollideCircle(16.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);


	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,192.f,0.f }, glm::vec3{ 640.f, 32.f,0.f }, "Object", ObjectType::WALL);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({320.f, 16.f});
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,-192.f,0.f }, glm::vec3{ 640.f, 32.f,0.f }, "Wall", ObjectType::WALL);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 320.f, 16.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 302.f, 0.f,0.f }, glm::vec3{ 32.f, 334.f,0.f }, "Wall", ObjectType::WALL);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 16.f, 176.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ -302.f, 0.f,0.f }, glm::vec3{ 32.f, 334.f,0.f }, "Wall", ObjectType::WALL);

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(1.f));
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 16.f, 176.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);
}

void PocketBallDemo::Update(float dt)
{
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::DOWN))
	{
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetComponent<Physics2D>()->AddForceY(20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::UP))
	{
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetComponent<Physics2D>()->AddForceY(-20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::LEFT))
	{
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetComponent<Physics2D>()->AddForceX(-1000.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetComponent<Physics2D>()->AddForceX(20.f);
	}
	CollideObjects();
}

#ifdef _DEBUG
void PocketBallDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
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
	//Engine::Instance().GetParticleManager()->Clear();
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
}

