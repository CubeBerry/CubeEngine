//Author: DOYEONG LEE
//Project: CubeEngine
//File: PPlayer.cpp
#include "PlatformDemo/PPlayer.hpp"
#include "PlatformDemo/PBullet.hpp"
#include "PlatformDemo/PEnemyBullet.hpp"
#include "PlatformDemo/PlatformDemoSystem.hpp"

#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "PlatformDemo/PlatformDemoSystem.hpp"

#include "Engine.hpp"

#include <iostream>

PPlayer::PPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
	: Object(pos_, size_, name, objectType)
{
}

PPlayer::PPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name, PlatformDemoSystem* sys)
	: Object(pos_, size_, name, ObjectType::PLAYER)
{
	AddComponent<Sprite>();
	GetComponent<Sprite>()->LoadAnimation("../Game/assets/PlatformDemo/player.spt", "Player");

	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->SetMinVelocity({ 0.01f, 0.1f });
	GetComponent<Physics2D>()->SetGravity(700.f);
	GetComponent<Physics2D>()->SetFriction(3.f);
	GetComponent<Physics2D>()->SetMaxVelocity({ 400.f,800.f });
	GetComponent<Physics2D>()->AddCollidePolygonAABB({ size_.x / 2.f,  size_.y / 2.f });
	GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);
	GetComponent<Sprite>()->PlayAnimation(0);
	platformDemoSystem = sys;
}


void PPlayer::Init()
{
}

void PPlayer::Update(float dt)
{
	std::cout << GetComponent<Physics2D>()->GetVelocity().y << std::endl;
	isGrounded = false;
	for (auto& obj : Engine::GetObjectManager().GetObjectMap())
	{
		if (obj.second->GetObjectType() == ObjectType::WALL)
		{
			if (GetPosition().y >= obj.second->GetPosition().y && GetComponent<Physics2D>()->CollisionPPWithoutPhysics(this, obj.second.get()))
			{
				isGrounded = true;
				SetStateOff(PlayerStates::JUMPING);
				SetStateOff(PlayerStates::FALLING);
				break;
			}
		}
	}

	Control(dt);
	Object::Update(dt);
	Jumping();


	if (canAttack == false)
	{
		if (GetComponent<Sprite>()->IsAnimationDone() == true)
		{
			canAttack = true;
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
	platformDemoSystem = nullptr;
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}

void PPlayer::CollideObject(Object* obj)
{
	switch (obj->GetObjectType())
	{
	case ObjectType::WALL:
		GetComponent<Physics2D>()->CheckCollision(obj);
		obj->GetComponent<Physics2D>()->CheckCollision(this);
		break;
	case ObjectType::ENEMYBULLET:
		if (GetInvincibleState() == false && GetComponent<Physics2D>()->CheckCollision(obj) == true)
		{
			PEnemyBullet* b = static_cast<PEnemyBullet*>(obj);

			platformDemoSystem->HpDecrease(b->GetDamage());
			SetInvincibleState(true);

			Engine::GetObjectManager().Destroy(b->GetId());
			b = nullptr;
		}
		break;
	}
}

void PPlayer::Control(float /*dt*/)
{
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::DOWN))
	{
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::UP))
	{
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::LEFT))
	{
		if (IsStateOn(PlayerStates::DIRECTION) == true)
		{
			Object::SetXSize(-Object::GetSize().x);
		}
		SetStateOff(PlayerStates::DIRECTION);
		GetComponent<Physics2D>()->AddForceX(-300.f);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		if (IsStateOn(PlayerStates::DIRECTION) == false)
		{
			Object::SetXSize(-Object::GetSize().x);
		}
		SetStateOn(PlayerStates::DIRECTION);
		GetComponent<Physics2D>()->AddForceX(300.f);
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::X)
		&& IsStateOn(PlayerStates::FALLING) == false && IsStateOn(PlayerStates::JUMPING) == false)
	{
		Object::SetYPosition(Object::GetPosition().y + 1.f);
		GetComponent<Physics2D>()->SetVelocityY(400.f);
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::Z))
	{
		if (canAttack == true)
		{
			{
				GetComponent<Sprite>()->PlayAnimation(3);
				canAttack = false;
			}

			Engine::GetObjectManager().AddObject<PBullet>(position, glm::vec3{ 8.f,8.f,0.f }, "Bullet");
			if (IsStateOn(PlayerStates::DIRECTION))
			{
				Engine::GetObjectManager().GetLastObject()->SetXSpeed(1000.f);
			}
			else
			{
				Engine::GetObjectManager().GetLastObject()->SetXSpeed(-1000.f);
			}
		}
	}
}

void PPlayer::Jumping()
{
	if (isGrounded == false)
	{
		if (GetComponent<Physics2D>()->GetVelocity().y > 0.f)
		{
			if (canAttack == true) GetComponent<Sprite>()->PlayAnimation(2);
			SetStateOn(PlayerStates::JUMPING);
			SetStateOff(PlayerStates::FALLING);
		}
		else if (GetComponent<Physics2D>()->GetVelocity().y < 0.f)
		{
			if (canAttack == true) GetComponent<Sprite>()->PlayAnimation(2);
			SetStateOn(PlayerStates::FALLING);
			SetStateOff(PlayerStates::JUMPING);
		}
	}
	else
	{
		if (canAttack == true)
		{
			if (abs(GetComponent<Physics2D>()->GetVelocity().x) < 5.f)
			{
				GetComponent<Sprite>()->PlayAnimation(0);
			}
			else
			{
				GetComponent<Sprite>()->PlayAnimation(1);
			}
		}
	}

}