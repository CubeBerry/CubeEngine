//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUPlayer.cpp

#include "BeatEmUpDemo/BEUPlayer.hpp"
#include "BeatEmUpDemo/BEUAttackBox.hpp"

#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "BeatEmUpDemo/BeatEmUpDemoSystem.hpp"

#include "Engine.hpp"

#include <iostream>

BEUPlayer::BEUPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
	: Object(pos_, size_, name, objectType)
{
}

BEUPlayer::BEUPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name, BeatEmUpDemoSystem* sys)
	: Object(pos_, size_, name, ObjectType::PLAYER)
{
	AddComponent<Sprite>();
	GetComponent<Sprite>()->LoadAnimation("../Game/assets/BeatEmUpDemo/player.spt", "Player");
	GetComponent<Sprite>()->PlayAnimation(0);

	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->SetMinVelocity({ 0.0f, 0.f });
	GetComponent<Physics2D>()->SetMaxVelocity({ 10.f,50.f });
	GetComponent<Physics2D>()->SetGravity(75.0f);
	GetComponent<Physics2D>()->SetFriction(0.f);
	GetComponent<Physics2D>()->AddCollidePolygonAABB({ size_.x / 4.f,  size_.y / 2.f });
	GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);
	beatEmUpDemoSystem = sys;
}


void BEUPlayer::Init()
{
}

void BEUPlayer::Update(float dt)
{
	Object::Update(dt);
	if (IsStateOn(BEUObjectStates::KNOCKBACK))
	{
		KnockBack(dt);
	}
	else if (beatEmUpDemoSystem->GetHp() <= 0.f)
	{
		isInvincible = true;
		SetStateOn(BEUObjectStates::KNOCKBACK);
		GetComponent<Physics2D>()->SetVelocityY(20.f);
		if (IsStateOn(BEUObjectStates::DIRECTION) == true) //Right
		{
			GetComponent<Physics2D>()->SetVelocityX(-7.f);
		}
		else //Left
		{
			GetComponent<Physics2D>()->SetVelocityX(7.f);
		}
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
				if (IsStateOn(BEUObjectStates::KNOCKBACK) == false)
				{
					GetComponent<Sprite>()->PlayAnimation(0);
				}
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
		if (attackDelay > 0.5f)
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
	glm::vec3 cameraPos = Engine::GetCameraManager().GetCameraPosition();
	if(position.x < cameraPos.x -11.f)
	{
		position.x = -11.f;
	}
	else if (position.x > cameraPos.x + 11.f)
	{
		position.x = 11.f;
	}
}

void BEUPlayer::End()
{
	beatEmUpDemoSystem = nullptr;
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}

void BEUPlayer::CollideObject(Object* obj)
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
			beatEmUpDemoSystem->HpDecrease(static_cast<BEUAttackBox*>(obj)->GetDamage());
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
			GetComponent<Sprite>()->PlayAnimation(7);

			switch (static_cast<BEUAttackBox*>(obj)->GetAttackBoxType())
			{
			case AttackBoxType::NORMAL:
				Engine::GetSoundManager().Play("hitNormal.wav", 0);
				SetStateOn(BEUObjectStates::HIT);
				if (IsStateOn(BEUObjectStates::FALLING) == true || IsStateOn(BEUObjectStates::JUMPING) == true)
				{
					isInvincible = true;
					SetStateOn(BEUObjectStates::KNOCKBACK);
					GetComponent<Physics2D>()->SetVelocityY(0.3f);
					if (IsStateOn(BEUObjectStates::MOVE) == true)
					{
						if (IsStateOn(BEUObjectStates::DIRECTION) == true) //Right
						{
							GetComponent<Physics2D>()->AddForceX(-10.f);
						}
						else //Left
						{
							GetComponent<Physics2D>()->AddForceX(10.f);
						}
					}
				}
				//Engine::GetObjectManager().Destroy(obj->GetId());
				break;
			case AttackBoxType::FINISH:
				Engine::GetSoundManager().Play("hitFinish.wav", 0);
				isInvincible = true;
				SetStateOn(BEUObjectStates::HIT);
				SetStateOn(BEUObjectStates::KNOCKBACK);
				if (IsStateOn(BEUObjectStates::FALLING) == true || IsStateOn(BEUObjectStates::JUMPING) == true)
				{
					GetComponent<Physics2D>()->SetVelocityY(20.f);
					if (IsStateOn(BEUObjectStates::DIRECTION) == true) //Right
					{
						GetComponent<Physics2D>()->SetVelocityX(-7.f);
					}
					else //Left
					{
						GetComponent<Physics2D>()->SetVelocityX(7.f);
					}
				}
				else
				{
					Object::SetYPosition(Object::GetPosition().y + 1.f);
					GetComponent<Physics2D>()->SetVelocityY(20.f);
					if (IsStateOn(BEUObjectStates::DIRECTION) == true) //Right
					{
						GetComponent<Physics2D>()->SetVelocityX(-7.f);
					}
					else //Left
					{
						GetComponent<Physics2D>()->SetVelocityX(7.f);
					}
				}
				//Engine::GetObjectManager().Destroy(obj->GetId());
				break;
			}
		}
		break;
	}
}

void BEUPlayer::Control(float dt)
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
		}
		if (GetComponent<Sprite>()->GetCurrentAnim() != 1)
		{
			GetComponent<Sprite>()->PlayAnimation(1);
		}
		SetZPosition(GetPosition().z + 10.f * dt);
		if (GetPosition().z > 5.f)
		{
			SetZPosition(GetPosition().z - 10.f * dt);
		}
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
		}
		if (GetComponent<Sprite>()->GetCurrentAnim() != 1)
		{
			GetComponent<Sprite>()->PlayAnimation(1);
		}
		SetZPosition(GetPosition().z - 10.f * dt);
		if (GetPosition().z < -30.f)
		{
			SetZPosition(GetPosition().z + 10.f * dt);
		}
	}
	if (Engine::GetInputManager().IsKeyReleaseOnce(KEYBOARDKEYS::LEFT) && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		SetStateOff(BEUObjectStates::MOVE);
		SetStateOff(BEUObjectStates::MOVEFOWARD);

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
		}
		if (GetComponent<Sprite>()->GetCurrentAnim() != 1)
		{
			GetComponent<Sprite>()->PlayAnimation(1);
		}
		SetStateOn(BEUObjectStates::MOVEFOWARD);
		SetStateOff(BEUObjectStates::DIRECTION);
		SetXPosition(GetPosition().x - 10.f * dt);
	}
	if (Engine::GetInputManager().IsKeyReleaseOnce(KEYBOARDKEYS::RIGHT) && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		SetStateOff(BEUObjectStates::MOVE);
		SetStateOff(BEUObjectStates::MOVEFOWARD);

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
		if (GetComponent<Sprite>()->GetCurrentAnim() != 1)
		{
			GetComponent<Sprite>()->PlayAnimation(1);
		}
		SetStateOn(BEUObjectStates::DIRECTION);
		SetStateOn(BEUObjectStates::MOVEFOWARD);
		SetXPosition(GetPosition().x + 10.f * dt);
		/*
		if (beatEmUpDemoSystem->GetIsCameraMoveAble() == true && Engine::GetCameraManager().GetCameraPosition().x < position.x)
		{
			Engine::GetCameraManager().SetCameraPosition({ position.x , Engine::GetCameraManager().GetCameraPosition().y, Engine::GetCameraManager().GetCameraPosition().z });
		}*/
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::X)
		&& IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		Object::SetYPosition(Object::GetPosition().y + 1.f);
		GetComponent<Physics2D>()->SetVelocityY(25.f);
		if (IsStateOn(BEUObjectStates::MOVEFOWARD) == true)
		{
			if (IsStateOn(BEUObjectStates::DIRECTION) == true) //Right
			{
				GetComponent<Physics2D>()->SetVelocityX(5.5f);
			}
			else //Left
			{
				GetComponent<Physics2D>()->SetVelocityX(-5.5f);
			}
		}
	}
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::Z) && IsStateOn(BEUObjectStates::ATTACK) == false)
	{
		if (canAttack == true)
		{
			if (IsStateOn(BEUObjectStates::FALLING) == false && IsStateOn(BEUObjectStates::JUMPING) == false)
			{
				Engine::GetSoundManager().Play("punch.wav", 0);
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
					Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ 1.5f, -1.5f , 0.f }
					, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.5f, AttackBoxType::FINISH);
				}
				else
				{
					Engine::GetObjectManager().AddObject<BEUAttackBox>(glm::vec3{ -1.5f, -1.5f , 0.f }
					, glm::vec3{ 1.5f, 1.f, 0.f }, "AttackBox", this, 0.5f, AttackBoxType::FINISH);
				}
			}
		}
	}
}

void BEUPlayer::SetIsAttackHit(bool state_)
{
	isAttackHit = state_;
	if (state_ == true)
	{
		attackDelay = 0.f;
		combo++;
	}
}

void BEUPlayer::Jumping()
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
	else if (GetPosition().y <= 0.f)
	{
		if (IsStateOn(BEUObjectStates::FALLING) == true)
		{
			SetStateOff(BEUObjectStates::FALLING);
			SetStateOff(BEUObjectStates::JUMPING);
			SetYPosition(0.f);
			{
				if (canAttack == false)
				{
					canAttack = true;
					SetStateOff(BEUObjectStates::ATTACK);
				}

				GetComponent<Sprite>()->PlayAnimation(0);
			}
		}
		SetYPosition(0.f);
		GetComponent<Physics2D>()->SetVelocityX(0.f);
		GetComponent<Physics2D>()->SetVelocityY(0.f);
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
}

void BEUPlayer::KnockBack(float dt)
{
	if (GetComponent<Physics2D>()->GetVelocity().y > 0.f)
	{
		if (IsStateOn(BEUObjectStates::JUMPING) == false)
		{
			GetComponent<Sprite>()->PlayAnimation(8);
			SetStateOn(BEUObjectStates::JUMPING);
			SetStateOff(BEUObjectStates::FALLING);
		}
	}
	else if (GetPosition().y <= 0.f)
	{
		if (IsStateOn(BEUObjectStates::FALLING) == true)
		{
			Engine::GetSoundManager().Play("knockdown.wav", 1);
			SetStateOff(BEUObjectStates::FALLING);
			SetStateOff(BEUObjectStates::JUMPING);
			SetStateOn(BEUObjectStates::LAYING);
			GetComponent<Sprite>()->PlayAnimation(9);

			SetYPosition(0.f);
		}
		if (IsStateOn(BEUObjectStates::LAYING) == true)
		{
			SetYPosition(0.f);
			layingDelay += dt;
			if (beatEmUpDemoSystem->GetHp() <= 0.f)
			{
				glm::vec4 color = GetComponent<Sprite>()->GetColor();
				if (color.w == 1.f)
				{
					GetComponent<Sprite>()->SetColor({ color.r, color.g, color.b, 0.f });
				}
				else
				{
					GetComponent<Sprite>()->SetColor({ color.r, color.g, color.b, 1.f });
				}
			}
			if (layingDelay > 1.25f)
			{
				if (beatEmUpDemoSystem->GetHp() > 0.f)
				{
					layingDelay = 0.f;
					hitDelay = 0.f;
					SetStateOff(BEUObjectStates::HIT);
					SetStateOff(BEUObjectStates::KNOCKBACK);
					SetStateOff(BEUObjectStates::LAYING);
					SetStateOff(BEUObjectStates::ATTACK);
					GetComponent<Sprite>()->PlayAnimation(0);
					isInvincible = false;
					canAttack = true;
				}
				else
				{
					Engine::GetGameStateManager().SetGameState(State::RESTART);
				}
			}
		}
		GetComponent<Physics2D>()->SetVelocityX(0.f);
		GetComponent<Physics2D>()->SetVelocityY(0.f);
	}
	else if (GetComponent<Physics2D>()->GetVelocity().y < 0.f)
	{
		if (IsStateOn(BEUObjectStates::FALLING) == false)
		{
			GetComponent<Sprite>()->PlayAnimation(8);
			SetStateOn(BEUObjectStates::FALLING);
			SetStateOff(BEUObjectStates::JUMPING);
		}
	}
}
