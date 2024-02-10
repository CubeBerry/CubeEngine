//Author: DOYEONG LEE
//Project: CubeEngine
//File: Ball.cpp
#include "PocketBallDemo/Ball.hpp"
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"

#include <iostream>
Ball::Ball(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
	: Object(pos_, size_, name, objectType)
{
}

Ball::Ball(glm::vec3 pos_, glm::vec3 size_, std::string name, BallType ballType_)
	: Object(pos_, size_, name, ObjectType::BALL)
{
	ballType = ballType_;
	switch (ballType)
	{
	case BallType::WHITE:
		AddComponent<Sprite>();
		GetComponent<Sprite>()->AddMeshWithTexture("White");
		break;
	case BallType::OTHER:
		AddComponent<Sprite>();
		GetComponent<Sprite>()->AddMeshWithTexture("Black");
		break;
	}
	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->SetRestitution(-1.f);
	GetComponent<Physics2D>()->SetMinVelocity({ 0.1f,0.1f });
	GetComponent<Physics2D>()->SetMass(1.f);
	GetComponent<Physics2D>()->SetFriction(0.99f);
	GetComponent<Physics2D>()->AddCollideCircle(size_.x / 2.f);
	GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);
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
}