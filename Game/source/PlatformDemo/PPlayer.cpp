//Author: DOYEONG LEE
//Project: CubeEngine
//File: PPlayer.cpp
#include "PlatformDemo/PPlayer.hpp"
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "Engine.hpp"

#include <iostream>
PPlayer::PPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
	: Object(pos_, size_, name, objectType)
{
}

PPlayer::PPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name)
	: Object(pos_, size_, name, ObjectType::PLAYER)
{
	AddComponent<Sprite>();
	GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,1.f });

	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->SetMinVelocity({ 0.1f, 0.1f });
	GetComponent<Physics2D>()->SetGravity(40.f);
	GetComponent<Physics2D>()->SetFriction(0.9f);
	GetComponent<Physics2D>()->SetMaxVelocity({ 10.f,20.f });
	GetComponent<Physics2D>()->AddCollidePolygonAABB({ size_.x / 2.f,  size_.y / 2.f });
	GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);
}


void PPlayer::Init()
{
}

void PPlayer::Update(float dt)
{
	GetComponent<Physics2D>()->Gravity(dt);
	Object::Update(dt);
	Control(dt);

	if (GetComponent<Physics2D>()->GetVelocity().y > 0.f)
	{
		SetStateOn(States::JUMPING);
		SetStateOff(States::FALLING);
	}
	else if (GetComponent<Physics2D>()->GetVelocity().y < 0.f)
	{
		SetStateOn(States::FALLING);
		SetStateOff(States::JUMPING);
	}
}

void PPlayer::End()
{
}

void PPlayer::Control(float dt)
{
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::DOWN))
	{
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::UP))
	{
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::LEFT))
	{
		GetComponent<Physics2D>()->AddForceX(-20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		GetComponent<Physics2D>()->AddForceX(20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressedOnce(KEYBOARDKEYS::SPACE)
		&& IsStateOn(States::ONGROUND) == true && IsStateOn(States::JUMPING) == false)
	{
		Object::SetYPosition(Object::GetPosition().y + 1.f);
		GetComponent<Physics2D>()->SetVelocityY(80.f);
		SetStateOff(States::ONGROUND);
	}
}
