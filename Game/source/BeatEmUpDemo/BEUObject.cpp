//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUObject.cpp

#include "BeatEmUpDemo/BEUObject.hpp"
#include "BeatEmUpDemo/BEUAttackBox.hpp"

#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "BeatEmUpDemo/BeatEmUpDemoSystem.hpp"

#include "Engine.hpp"

#include <iostream>

BEUObject::BEUObject(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
	: Object(pos_, size_, name, objectType)
{
}

BEUObject::BEUObject(glm::vec3 pos_, glm::vec3 size_, std::string name, BeatEmUpDemoSystem* sys)
	: Object(pos_, size_, name, ObjectType::NONE)
{
	AddComponent<Sprite>();
	GetComponent<Sprite>()->LoadAnimation("../Game/assets/BeatEmUpDemo/player.spt", "Player");
	GetComponent<Sprite>()->PlayAnimation(0);

	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->SetMinVelocity({ 0.01f, 0.1f });
	GetComponent<Physics2D>()->SetGravity(15.f);
	GetComponent<Physics2D>()->SetFriction(0.9f);
	GetComponent<Physics2D>()->SetMaxVelocity({ 20.f,5.f });
	GetComponent<Physics2D>()->AddCollidePolygonAABB({ size_.x / 2.f,  size_.y / 2.f });
	GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);
	beatEmUpDemoSystem = sys;
}


void BEUObject::Init()
{
}

void BEUObject::Update(float dt)
{
	Object::Update(dt);
	GetComponent<Physics2D>()->Gravity(dt);
	if (IsStateOn(BEUObjectStates::KNOCKBACK))
	{
		KnockBack(dt);
	}
	else
	{
		Jumping();
		if (IsStateOn(BEUObjectStates::HIT) == false)
		{
			Control(dt);
		}
		else
		{
			hitDelay += dt;
			if (hitDelay > 0.6f)
			{
				hitDelay = 0.f;
				SetStateOff(BEUObjectStates::HIT);
			}
		}
	}

	if (canAttack == false)
	{
		if (GetComponent<Sprite>()->IsAnimationDone() == true)
		{
			canAttack = true;
			SetStateOff(BEUObjectStates::ATTACK);
			GetComponent<Sprite>()->PlayAnimation(0);
		}
	}
	if (isAttackHit == true)
	{
		attackDelay += dt;
		if (attackDelay > 0.4)
		{
			combo = 0;
			attackDelay = 0.f;
			isAttackHit = false;
		}
	}

	if (isInvincible == true)
	{
		invincibleDelay += dt;
		if (invincibleDelay > 0.75f)
		{
			isInvincible = false;
			invincibleDelay = 0.f;
		}
	}
}

void BEUObject::End()
{
	beatEmUpDemoSystem = nullptr;
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}

void BEUObject::CollideObject(Object* obj)
{
	switch (obj->GetObjectType())
	{
	case ObjectType::WALL:
		GetComponent<Physics2D>()->CheckCollision(obj);
		obj->GetComponent<Physics2D>()->CheckCollision(this);
		break;
	case ObjectType::ENEMYBULLET:
		if (GetInvincibleState() == false && GetComponent<Physics2D>()->CheckCollision(obj) == true && (position.z < obj->GetPosition().z + 1.f && position.z > obj->GetPosition().z - 1.f))
		{
			if (IsStateOn(BEUObjectStates::DIRECTION) == true && obj->GetPosition().x <= position.x)
			{
				SetStateOff(BEUObjectStates::DIRECTION);
				Object::SetXSize(-Object::GetSize().x);
			}
			else if (IsStateOn(BEUObjectStates::DIRECTION) == false && obj->GetPosition().x > position.x)
			{
				SetStateOn(BEUObjectStates::DIRECTION);
				Object::SetXSize(-Object::GetSize().x);
			}

			switch (static_cast<BEUAttackBox*>(obj)->GetAttackBoxType())
			{
			case AttackBoxType::NORMAL:
				SetStateOn(BEUObjectStates::HIT);
				if (IsStateOn(BEUObjectStates::FALLING) == true || IsStateOn(BEUObjectStates::JUMPING) == true)
				{
					SetStateOn(BEUObjectStates::KNOCKBACK);
					GetComponent<Physics2D>()->SetVelocityY(1.25f);
					if (IsStateOn(BEUObjectStates::MOVE) == true)
					{
						if (IsStateOn(BEUObjectStates::DIRECTION) == true) //Right
						{
							GetComponent<Physics2D>()->AddForceX(-20.f);
						}
						else //Left
						{
							GetComponent<Physics2D>()->AddForceX(20.f);
						}
					}
				}
				Engine::GetObjectManager().Destroy(obj->GetId());
				break;
			case AttackBoxType::FINISH:
				SetStateOn(BEUObjectStates::HIT);
				SetStateOn(BEUObjectStates::KNOCKBACK);
				if (IsStateOn(BEUObjectStates::FALLING) == true || IsStateOn(BEUObjectStates::JUMPING) == true)
				{
					GetComponent<Physics2D>()->SetVelocityY(1.25f);
				}
				else
				{
					Object::SetYPosition(Object::GetPosition().y + 1.f);
					GetComponent<Physics2D>()->SetVelocityY(2.5f);
				}
				if (IsStateOn(BEUObjectStates::MOVE) == true)
				{
					if (IsStateOn(BEUObjectStates::DIRECTION) == true) //Right
					{
						GetComponent<Physics2D>()->AddForceX(-20.f);
					}
					else //Left
					{
						GetComponent<Physics2D>()->AddForceX(20.f);
					}
				}
				Engine::GetObjectManager().Destroy(obj->GetId());
				break;
			}
		}
		break;
	}
}

void BEUObject::Control(float dt)
{
	if (Engine::GetInputManager().IsKeyReleaseOnce(KEYBOARDKEYS::DOWN)
		&& IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		SetStateOff(BEUObjectStates::MOVE);
		GetComponent<Sprite>()->PlayAnimation(0);
	}
	else if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::DOWN)
		&& IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		if (IsStateOn(BEUObjectStates::MOVE) == false)
		{
			SetStateOn(BEUObjectStates::MOVE);
			GetComponent<Sprite>()->PlayAnimation(1);
		}
		SetZPosition(GetPosition().z + 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyReleaseOnce(KEYBOARDKEYS::UP)
		&& IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		GetComponent<Sprite>()->PlayAnimation(0);
		SetStateOff(BEUObjectStates::MOVE);
	}
	else if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::UP)
		&& IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		if (IsStateOn(BEUObjectStates::MOVE) == false)
		{
			SetStateOn(BEUObjectStates::MOVE);
			GetComponent<Sprite>()->PlayAnimation(1);
		}
		SetZPosition(GetPosition().z - 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyReleaseOnce(KEYBOARDKEYS::LEFT) && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		std::cout << "LeftOFF" << std::endl;
		SetStateOff(BEUObjectStates::MOVE);

		if (IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false)
			GetComponent<Sprite>()->PlayAnimation(0);
	}
	else if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::LEFT)
		&& IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		if (IsStateOn(BEUObjectStates::DIRECTION) == true)
		{
			Object::SetXSize(-Object::GetSize().x);
		}
		if (IsStateOn(BEUObjectStates::MOVE) == false)
		{
			SetStateOn(BEUObjectStates::MOVE);
			GetComponent<Sprite>()->PlayAnimation(1);
		}
		SetStateOff(BEUObjectStates::DIRECTION);
		SetXPosition(GetPosition().x - 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyReleaseOnce(KEYBOARDKEYS::RIGHT) && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		std::cout << "RightOFF" << std::endl;
		SetStateOff(BEUObjectStates::MOVE);

		if (IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false)
			GetComponent<Sprite>()->PlayAnimation(0);
	}
	else if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::RIGHT)
		&& IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		if (IsStateOn(BEUObjectStates::DIRECTION) == false)
		{
			Object::SetXSize(-Object::GetSize().x);
		}
		if (IsStateOn(BEUObjectStates::MOVE) == false)
		{
			SetStateOn(BEUObjectStates::MOVE);
			GetComponent<Sprite>()->PlayAnimation(1);
		}
		SetStateOn(BEUObjectStates::DIRECTION);
		SetXPosition(GetPosition().x + 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::X)
		&& IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		Object::SetYPosition(Object::GetPosition().y + 1.f);
		GetComponent<Physics2D>()->SetVelocityY(2.5f);
		if (IsStateOn(BEUObjectStates::MOVE) == true)
		{
			if (IsStateOn(BEUObjectStates::DIRECTION) == true) //Right
			{
				GetComponent<Physics2D>()->AddForceX(20.f);
			}
			else //Left
			{
				GetComponent<Physics2D>()->AddForceX(-20.f);
			}
		}
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::Z) && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		if (canAttack == true)
		{
			if (IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false)
			{
				switch (combo)
				{
				case 0:
					GetComponent<Sprite>()->PlayAnimation(3);
					canAttack = false;
					SetStateOn(BEUObjectStates::ATTACK);
					if (IsStateOn(BEUObjectStates::DIRECTION) == true)
					{
						Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ 1.f, 0.5f , 0.f }
						, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.1f);
					}
					else
					{
						Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ -1.f, 0.5f , 0.f }
						, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.1f);
					}
					break;
				case 1:
					GetComponent<Sprite>()->PlayAnimation(3);
					canAttack = false;
					SetStateOn(BEUObjectStates::ATTACK);
					if (IsStateOn(BEUObjectStates::DIRECTION) == true)
					{
						Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ 1.f, 0.5f , 0.f }
						, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.1f);
					}
					else
					{
						Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ -1.f, 0.5f , 0.f }
						, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.1f);
					}
					break;
				case 2:
					GetComponent<Sprite>()->PlayAnimation(4);
					canAttack = false;
					SetStateOn(BEUObjectStates::ATTACK);
					if (IsStateOn(BEUObjectStates::DIRECTION) == true)
					{
						Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ 1.f, 0.5f , 0.f }
						, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.1f);
					}
					else
					{
						Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ -1.f, 0.5f , 0.f }
						, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.1f);
					}
					break;
				case 3:
					GetComponent<Sprite>()->PlayAnimation(5);
					canAttack = false;
					SetStateOn(BEUObjectStates::ATTACK);
					if (IsStateOn(BEUObjectStates::DIRECTION) == true)
					{
						Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ 1.25f, 1.f , 0.f }
						, glm::vec3{ 1.f, 2.f, 0.f }, "AttackBox", this, 0.25f, AttackBoxType::FINISH);
					}
					else
					{
						Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ -1.25f, 1.f , 0.f }
						, glm::vec3{ 1.f, 2.f, 0.f }, "AttackBox", this, 0.2f, AttackBoxType::FINISH);
					}
					break;
				}
			}
			else
			{
				GetComponent<Sprite>()->PlayAnimation(6);
				canAttack = false;
				SetStateOn(BEUObjectStates::ATTACK);
				if (IsStateOn(BEUObjectStates::DIRECTION) == true)
				{
					Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ 1.f, -1.f , 0.f }
					, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.1f, AttackBoxType::FINISH);
				}
				else
				{
					Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ -1.f, -1.f , 0.f }
					, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.1f, AttackBoxType::FINISH);
				}
			}
		}
	}
}

void BEUObject::SetIsAttackHit(bool state_)
{
	isAttackHit = state_;
	if (state_ == true)
	{
		attackDelay = 0.f;
		std::cout << combo << std::endl;
		combo++;
	}
}

void BEUObject::Jumping()
{
	if (GetComponent<Physics2D>()->GetVelocity().y > 0.f)
	{
		if (IsStateOn(BEUObjectStates::JUMPING) == false)
		{
			if (canAttack == true)
			{
				GetComponent<Sprite>()->PlayAnimation(2);
			}
			else
			{
				GetComponent<Sprite>()->PlayAnimation(6);
			}

			SetStateOn(BEUObjectStates::JUMPING);
			SetStateOff(BEUObjectStates::FALLING);
		}
	}
	else if (GetComponent<Physics2D>()->GetVelocity().y > -0.9f &&
		GetComponent<Physics2D>()->GetVelocity().y < 0.0f)
	{
		if (IsStateOn(BEUObjectStates::FALLING) == true)
		{
			SetStateOff(BEUObjectStates::FALLING);
			SetStateOff(BEUObjectStates::JUMPING);
			SetYPosition(0.f);
			if (GetComponent<Physics2D>()->GetVelocity().x < 0.5f ||
				GetComponent<Physics2D>()->GetVelocity().x > -0.5f)
			{
				if (canAttack == false)
				{
					canAttack = true;
					SetStateOff(BEUObjectStates::ATTACK);
				}

				if (IsStateOn(BEUObjectStates::MOVE) == true)
				{
					GetComponent<Sprite>()->PlayAnimation(1);
				}
				else
				{
					GetComponent<Sprite>()->PlayAnimation(0);
				}
			}
			else
			{
				GetComponent<Sprite>()->PlayAnimation(0);
			}
		}
	}
	else if (GetComponent<Physics2D>()->GetVelocity().y < 0.f)
	{
		if (canAttack == true)
		{
			GetComponent<Sprite>()->PlayAnimation(2);
		}

		if (IsStateOn(BEUObjectStates::FALLING) == false)
		{
			SetStateOn(BEUObjectStates::FALLING);
			SetStateOff(BEUObjectStates::JUMPING);
		}
	}

	if (GetPosition().y <= 0.f)
	{
		GetComponent<Physics2D>()->SetVelocityY(0.f);
	}
}

void BEUObject::KnockBack(float dt)
{
	if (GetComponent<Physics2D>()->GetVelocity().y > 0.f)
	{
		if (IsStateOn(BEUObjectStates::JUMPING) == false)
		{
			SetStateOn(BEUObjectStates::JUMPING);
			SetStateOff(BEUObjectStates::FALLING);
		}
	}
	else if (GetComponent<Physics2D>()->GetVelocity().y > -0.9f &&
		GetComponent<Physics2D>()->GetVelocity().y < 0.0f)
	{
		if (IsStateOn(BEUObjectStates::FALLING) == true)
		{
			SetStateOff(BEUObjectStates::FALLING);
			SetStateOff(BEUObjectStates::JUMPING);
			SetStateOn(BEUObjectStates::LAYING);

			SetYPosition(0.f);
			if (GetComponent<Physics2D>()->GetVelocity().x < 0.5f ||
				GetComponent<Physics2D>()->GetVelocity().x > -0.5f)
			{
			
			}
		}
		if (IsStateOn(BEUObjectStates::LAYING) == true)
		{
			layingDelay += dt;
				if (layingDelay > 1.25f)
				{
					layingDelay = 0.f;
					hitDelay = 0.f;
					SetStateOff(BEUObjectStates::HIT);
					SetStateOff(BEUObjectStates::KNOCKBACK);
					SetStateOff(BEUObjectStates::LAYING);
					GetComponent<Sprite>()->PlayAnimation(0);
				}
		}
	}
	else if (GetComponent<Physics2D>()->GetVelocity().y < 0.f)
	{
		if (IsStateOn(BEUObjectStates::FALLING) == false)
		{
			SetStateOn(BEUObjectStates::FALLING);
			SetStateOff(BEUObjectStates::JUMPING);
		}
	}

	if (GetPosition().y <= 0.f)
	{
		GetComponent<Physics2D>()->SetVelocityY(0.f);
	}
}
