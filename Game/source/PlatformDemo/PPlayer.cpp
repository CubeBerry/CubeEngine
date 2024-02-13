//Author: DOYEONG LEE
//Project: CubeEngine
//File: PPlayer.cpp
#include "PlatformDemo/PPlayer.hpp"
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "PlatformDemo/PlatformDemoSystem.hpp"
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
	GetComponent<Physics2D>()->SetMinVelocity({ 0.01f, 0.1f });
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
	Object::Update(dt);
	GetComponent<Physics2D>()->Gravity(dt);
	Control(dt);

	if (GetComponent<Physics2D>()->GetVelocity().y > 0.f)
	{
		if (IsStateOn(States::JUMPING) == false)
		{
			SetStateOn(States::JUMPING);
			SetStateOff(States::FALLING);
			std::cout << "JUMP" << std::endl;
		}
		GetComponent<Sprite>()->SetColor({ 1.f,0.f,0.f,1.f });
	}
	else if (GetComponent<Physics2D>()->GetVelocity().y > -0.9f &&
		GetComponent<Physics2D>()->GetVelocity().y < 0.0f)
	{
		if (IsStateOn(States::FALLING) == true)
		{
			SetStateOff(States::FALLING);
			SetStateOff(States::JUMPING);
			std::cout << "ONGROUND" << std::endl;
		}
		GetComponent<Sprite>()->SetColor({ 1.f,1.f,1.f,1.f });
	}
	else if (GetComponent<Physics2D>()->GetVelocity().y < 0.f)
	{
		if (IsStateOn(States::FALLING) == false)
		{
			SetStateOn(States::FALLING);
			SetStateOff(States::JUMPING);
			std::cout << "FALL" << std::endl;
		}
		GetComponent<Sprite>()->SetColor({ 0.f,0.f,1.f,1.f });
	}
}

void PPlayer::End()
{
	Engine::Instance().GetParticleManager()->Clear();
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
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
		&& IsStateOn(States::FALLING) == false && IsStateOn(States::JUMPING) == false)
	{
		Object::SetYPosition(Object::GetPosition().y + 1.f);
		GetComponent<Physics2D>()->SetVelocityY(40.f);
	}
}
