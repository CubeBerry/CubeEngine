//Author: DOYEONG LEE
//Project: CubeEngine
//File: PPlayer.cpp
#include "PlatformDemo/PPlayer.hpp"
#include "PlatformDemo/PBullet.hpp"

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
	Engine::GetCameraManager()->SetCenter({ Object::position.x, 0.f, 0.f});
	Object::Update(dt);
	GetComponent<Physics2D>()->Gravity(dt);
	Control(dt);
	Jumping();

	if (canAttack == false)
	{
		attackDelay += dt;
		if (attackDelay > 1.f)
		{
			canAttack = true;
			attackDelay = 0.f;
		}
	}

	if (isInvincible == true)
	{
		invincibleDelay += dt;
		if (invincibleDelay > 1.f)
		{
			isInvincible = false;
			invincibleDelay = 0.f;
		}
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
		SetStateOff(States::DIRECTION);
		GetComponent<Physics2D>()->AddForceX(-20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		SetStateOn(States::DIRECTION);
		GetComponent<Physics2D>()->AddForceX(20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressedOnce(KEYBOARDKEYS::X)
		&& IsStateOn(States::FALLING) == false && IsStateOn(States::JUMPING) == false)
	{
		Object::SetYPosition(Object::GetPosition().y + 1.f);
		GetComponent<Physics2D>()->SetVelocityY(40.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressedOnce(KEYBOARDKEYS::Z))
	{
		if (canAttack == true)
		{
			Engine::GetObjectManager()->AddObject<PBullet>(position, glm::vec3{ 8.f,8.f,0.f }, "Bullet");
			if (IsStateOn(States::DIRECTION))
			{
				Engine::GetObjectManager()->GetLastObject()->SetXSpeed(1000.f);
			}
			else
			{
				Engine::GetObjectManager()->GetLastObject()->SetXSpeed(-1000.f);
			}
		}
	}
}

void PPlayer::Jumping()
{
	if (GetComponent<Physics2D>()->GetVelocity().y > 0.f)
	{
		if (IsStateOn(States::JUMPING) == false)
		{
			SetStateOn(States::JUMPING);
			SetStateOff(States::FALLING);
			std::cout << "JUMP" << std::endl;
		}
		GetComponent<Sprite>()->SetColor({ 1.f,0.f,0.f,1.f - invincibleDelay });
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
		GetComponent<Sprite>()->SetColor({ 1.f,1.f,1.f,1.f - invincibleDelay });
	}
	else if (GetComponent<Physics2D>()->GetVelocity().y < 0.f)
	{
		if (IsStateOn(States::FALLING) == false)
		{
			SetStateOn(States::FALLING);
			SetStateOff(States::JUMPING);
			std::cout << "FALL" << std::endl;
		}
		GetComponent<Sprite>()->SetColor({ 0.f,0.f,1.f,1.f - invincibleDelay });
	}
}