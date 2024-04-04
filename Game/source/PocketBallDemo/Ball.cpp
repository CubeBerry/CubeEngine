//Author: DOYEONG LEE
//Project: CubeEngine
//File: Ball.cpp
#include "PocketBallDemo/Ball.hpp"
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "PocketBallDemo/PocketBallSystem.hpp"

#include "Engine.hpp"
#include <iostream>
Ball::Ball(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
	: Object(pos_, size_, name, objectType)
{
}

Ball::Ball(glm::vec3 pos_, glm::vec3 size_, std::string name, BallType ballType_, PocketBallSystem* sys)
	: Object(pos_, size_, name, ObjectType::BALL)
{
	ballType = ballType_;
	switch (ballType)
	{
	case BallType::WHITE:
		AddComponent<Sprite>();
		GetComponent<Sprite>()->AddMeshWithTexture(name);
		break;
	case BallType::OTHER:
		AddComponent<Sprite>();
		GetComponent<Sprite>()->AddMeshWithTexture(name);
		break;
	}
	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->SetRestitution(-1.f);
	GetComponent<Physics2D>()->SetMinVelocity({ 0.2f,0.2f });
	GetComponent<Physics2D>()->SetMass(1.f);
	GetComponent<Physics2D>()->SetFriction(0.99f);
	GetComponent<Physics2D>()->AddCollideCircle(size_.x / 2.f);
	GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);
	pocketBallSystem = sys;
}

void Ball::Init()
{
}

void Ball::Update(float dt)
{
	Object::Update(dt);
}

void Ball::End()
{
	pocketBallSystem = nullptr;
}

void Ball::CollideObject(Object* obj)
{
	switch (obj->GetObjectType())
	{
	case ObjectType::WALL:
		GetComponent<Physics2D>()->CheckCollision(obj);
		obj->GetComponent<Physics2D>()->CheckCollision(this);
		break;
	case ObjectType::BALL:
		GetComponent<Physics2D>()->CheckCollision(obj);
		break;
	case ObjectType::GOAL:
		if (obj->GetComponent<Physics2D>()->CheckCollision(this) == true)
		{
			int time = (rand() % (3 + 1)) + 1;
			int amount = (rand() % (8 + 4)) + 4;
			int colorR = (rand() % (10 + 0)) + 0;
			int colorG = (rand() % (10 + 0)) + 0;
			int colorB = (rand() % (10 + 0)) + 0;
			int colorA = (rand() % (10 + 5)) + 5;
			float x = position.x;
			float y = position.y;
			float speedX = (float)(rand() % (15 - (-30) + 1) - 15);
			float speedY = (float)(rand() % (15 - (-30) + 1) - 15);

			Engine::GetParticleManager().AddRandomParticle({ x,y,0.f }, { 4.f,4.f,0.f }, { speedX,speedY,0.f }, 0.f, static_cast<float>(time), amount,
				{ static_cast<float>(colorR * 0.1f),static_cast<float>(colorG * 0.1f),static_cast<float>(colorB * 0.1f),static_cast<float>(colorA * 0.1f) });

			Engine::GetObjectManager().Destroy(Object::id);
			pocketBallSystem->SetBallNum(pocketBallSystem->GetBallNum() - 1);
		}
		break;
	}
}
