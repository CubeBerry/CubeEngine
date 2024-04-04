//Author: DOYEONG LEE
//Project: CubeEngine
//File: PEnemy.cpp
#include "PlatformDemo/PEnemy.hpp"
#include "PlatformDemo/PEnemyBullet.hpp"
#include "PlatformDemo/PBullet.hpp"

#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "Engine.hpp"

PEnemy::PEnemy(glm::vec3 pos_, glm::vec3 size_, std::string name, EnemyType type)
	: Object(pos_, size_, name, ObjectType::ENEMY)
{
	eType = type;
	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->SetMinVelocity({ 0.01f, 0.1f });
	GetComponent<Physics2D>()->SetGravity(40.f);
	GetComponent<Physics2D>()->SetFriction(0.9f);
	GetComponent<Physics2D>()->SetMaxVelocity({ 10.f,20.f });
	GetComponent<Physics2D>()->AddCollidePolygonAABB(size_ / 2.f);
	GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);

	AddComponent<Sprite>();
	switch (eType)
	{
	case EnemyType::NORMAL:
		GetComponent<Sprite>()->LoadAnimation("../Game/assets/PlatformDemo/enemy.spt", "Enemy");
		GetComponent<Sprite>()->PlayAnimation(0);
		hp = 1.f;
		break;
	case EnemyType::AIRSHIP:
		GetComponent<Sprite>()->LoadAnimation("../Game/assets/PlatformDemo/vehicle.spt", "Vehicle");
		GetComponent<Sprite>()->PlayAnimation(0);

		GetComponent<Physics2D>()->SetGhostCollision(true);
		GetComponent<Physics2D>()->SetMaxVelocity({ 5.f,4.f });
		GetComponent<Physics2D>()->SetFriction(1.f);
		size.x = -size.x;
		hp = 2.f;
		break;
	case EnemyType::BIG:
		GetComponent<Sprite>()->LoadAnimation("../Game/assets/PlatformDemo/bigEnemy.spt", "BigEnemy");
		GetComponent<Sprite>()->PlayAnimation(0);
		hp = 3.f;

		Object::SetXSize(-Object::GetSize().x);
		SetStateOff(EnemyStates::DIRECTION);
		break;
	case EnemyType::NONE:
		GetComponent<Sprite>()->AddQuad({ 1.f,0.f,0.f,1.f });
		break;
	}
}

void PEnemy::Init()
{

}

void PEnemy::Update(float dt)
{
	switch (eType)
	{
	case EnemyType::NORMAL:
		UpdateEnemyNormal(dt);
		break;
	case EnemyType::BIG:
		UpdateEnemyBig(dt);
		break;
	case EnemyType::AIRSHIP:
		UpdateEnemyAirShip(dt);
		break;
	}
}

void PEnemy::CollideObject(Object* obj)
{
	switch (obj->GetObjectType())
	{
	case ObjectType::WALL:
		GetComponent<Physics2D>()->CheckCollision(obj);
		obj->GetComponent<Physics2D>()->CheckCollision(this);
		break;
	case ObjectType::BULLET:
		if (GetInvincibleState() == false && GetComponent<Physics2D>()->CheckCollision(obj) == true)
		{
			PBullet* b = static_cast<PBullet*>(obj);

			SetHp(GetHp() - b->GetDamage());
			SetIsHit(true);

			Engine::GetObjectManager().Destroy(b->GetId());
			b = nullptr;
		}
		break;
	}
}

void PEnemy::SetAngleToTarget(Object* target)
{
	if (target != nullptr)
	{
		float dx = target->GetPosition().x - position.x;
		float dy = target->GetPosition().y - position.y;

		int angleToTarget = static_cast<int>(atan2(dy, dx) * -60.f);

		angle = static_cast<float>(angleToTarget - (angleToTarget % 5));
	}
}

void PEnemy::MoveTowardAngle(float dt)
{
	float valX = (cos(GetRotate() / 60.f) * GetSpeed().x);
	float valY = (sin(GetRotate() / 60.f) * GetSpeed().y);

	SetXPosition(GetPosition().x + valX * dt);
	SetYPosition(GetPosition().y - valY * dt);
}

void PEnemy::Hit(float dt)
{
	if (isHit == true)
	{
		GetComponent<Sprite>()->SetColor({ 0.6f,0.6f,0.6f,1.f });
		invincibleDelay += dt;
		isInvincible = true;
		if (invincibleDelay >= maxInvincibleDelay)
		{
			GetComponent<Sprite>()->SetColor({ 1.f,1.f,1.f,1.f });
			invincibleDelay = 0.f;
			isHit = false;
			isInvincible = false;
		}
	}
}

void PEnemy::UpdateEnemyNormal(float dt)
{
	if (hp < 0.f)
	{
		if (IsStateOn(EnemyStates::DEATH) == false)
		{
			isInvincible = true;
			SetStateOn(EnemyStates::DEATH);
			GetComponent<Sprite>()->PlayAnimation(3);
		}
		else
		{
			if (GetComponent<Sprite>()->IsAnimationDone() == true)
			{
				Engine::GetObjectManager().Destroy(Object::id);
			}
		}
	}

	Object::Update(dt);
	if (IsStateOn(EnemyStates::DEATH) == false)
	{
		Hit(dt);

		if (Engine::GetCameraManager().IsInCamera(this) == true)
		{
			GetComponent<Physics2D>()->Gravity(dt);
		}

		/*if (Engine::GetObjectManager().FindObjectWithName("Player") != nullptr)
		{
			Object* pTemp = Engine::GetObjectManager().FindObjectWithName("Player");
			if (pTemp->GetPosition().x > Object::position.x + abs(Object::size.x) / 3.f)
			{
				if (IsStateOn(EnemyStates::DIRECTION) == false)
				{
					Object::SetXSize(-Object::GetSize().x);
					SetStateOn(EnemyStates::DIRECTION);
				}
			}
			else if (pTemp->GetPosition().x < Object::position.x - abs(Object::size.x) / 3.f)
			{
				if (IsStateOn(EnemyStates::DIRECTION) == true)
				{
					Object::SetXSize(-Object::GetSize().x);
					SetStateOff(EnemyStates::DIRECTION);
				}
			}
			pTemp = nullptr;
		}*/
		if (Engine::GetObjectManager().FindObjectWithName("Player")->GetPosition().x > Object::position.x + abs(Object::size.x) / 3.f)
		{
			if (IsStateOn(EnemyStates::DIRECTION) == false)
			{
				Object::SetXSize(-Object::GetSize().x);
				SetStateOn(EnemyStates::DIRECTION);
			}
		}
		else if (Engine::GetObjectManager().FindObjectWithName("Player")->GetPosition().x < Object::position.x - abs(Object::size.x) / 3.f)
		{
			if (IsStateOn(EnemyStates::DIRECTION) == true)
			{
				Object::SetXSize(-Object::GetSize().x);
				SetStateOff(EnemyStates::DIRECTION);
			}
		}

		if (IsStateOn(EnemyStates::ATTACK) == false)
		{
			attackDelay += dt;
			if (attackDelay > 2.f)
			{
				Engine::GetObjectManager().AddObject<PEnemyBullet>(position, glm::vec3{ 8.f,8.f,0.f }, "EBullet");

				if (IsStateOn(EnemyStates::DIRECTION) == true)
				{
					Engine::GetObjectManager().GetLastObject()->SetXSpeed(1000.f);
				}
				else
				{
					Engine::GetObjectManager().GetLastObject()->SetXSpeed(-1000.f);
				}
				GetComponent<Sprite>()->PlayAnimation(2);
				SetStateOn(EnemyStates::ATTACK);
				attackDelay = 0.f;
			}
		}
		else
		{
			if (GetComponent<Sprite>()->IsAnimationDone() == true)
			{
				SetStateOff(EnemyStates::ATTACK);
				GetComponent<Sprite>()->PlayAnimation(0);
			}
		}
	}
}

void PEnemy::UpdateEnemyBig(float dt)
{
	if (hp < 0.f)
	{
		if (IsStateOn(EnemyStates::DEATH) == false)
		{
			isInvincible = true;
			SetStateOn(EnemyStates::DEATH);
			GetComponent<Sprite>()->PlayAnimation(4);
		}
		else
		{
			if (GetComponent<Sprite>()->IsAnimationDone() == true)
			{
				Engine::GetObjectManager().Destroy(Object::id);
			}
		}
	}

	Object::Update(dt);
	if (IsStateOn(EnemyStates::DEATH) == false)
	{
		Hit(dt);
		if (IsStateOn(EnemyStates::ONGROUND) == true)
		{
			glm::vec3 playerPos = Engine::GetObjectManager().FindObjectWithName("Player")->GetPosition();
			if (Engine::GetCameraManager().IsInCamera(this) == true)
			{
				GetComponent<Physics2D>()->Gravity(dt);
			}

			if (IsStateOn(EnemyStates::ATTACK) == false)
			{
				if (playerPos.x > Object::position.x - 512.f && playerPos.x < Object::position.x - abs(Object::size.x) / 3.f)
				{
					SetStateOn(EnemyStates::MOVE);
					SetStateOn(EnemyStates::TARGETFOUND);
					if (IsStateOn(EnemyStates::DIRECTION) == true)
					{
						Object::SetXSize(-Object::GetSize().x);
						SetStateOff(EnemyStates::DIRECTION);
					}
				}
				else if (playerPos.x < Object::position.x + 512.f && playerPos.x > Object::position.x + abs(Object::size.x) / 3.f)
				{
					SetStateOn(EnemyStates::MOVE);
					SetStateOn(EnemyStates::TARGETFOUND);
					if (IsStateOn(EnemyStates::DIRECTION) == false)
					{
						Object::SetXSize(-Object::GetSize().x);
						SetStateOn(EnemyStates::DIRECTION);
					}
				}
				else if ((playerPos.x > Object::position.x + 2.f && playerPos.x < Object::position.x + abs(Object::size.x) / 3.f))
				{
					SetStateOff(EnemyStates::MOVE);
					SetStateOn(EnemyStates::TARGETFOUND);
					if (GetComponent<Sprite>()->GetCurrentAnim() != 0)
					{
						GetComponent<Sprite>()->PlayAnimation(0);
					}
				}
				else if ((playerPos.x < Object::position.x - 2.f && playerPos.x > Object::position.x - abs(Object::size.x) / 3.f))
				{
					SetStateOff(EnemyStates::MOVE);
					SetStateOn(EnemyStates::TARGETFOUND);
					if (GetComponent<Sprite>()->GetCurrentAnim() != 0)
					{
						GetComponent<Sprite>()->PlayAnimation(0);
					}
				}
				else
				{
					if (IsStateOn(EnemyStates::MOVE) == true)
					{
						SetStateOff(EnemyStates::TARGETFOUND);
						GetComponent<Sprite>()->PlayAnimation(0);
						SetStateOff(EnemyStates::MOVE);
						attackDelay = 0.f;
					}
				}
			}

			if (IsStateOn(EnemyStates::ATTACK) == false && IsStateOn(EnemyStates::TARGETFOUND) == true)
			{
				if (IsStateOn(EnemyStates::MOVE) == true)
				{
					if (GetComponent<Sprite>()->GetCurrentAnim() != 1)
					{
						GetComponent<Sprite>()->PlayAnimation(1);
					}

					if (IsStateOn(EnemyStates::DIRECTION) == false)
					{
						GetComponent<Physics2D>()->SetVelocityX(-1.5f);
					}
					else
					{
						GetComponent<Physics2D>()->SetVelocityX(1.5f);
					}
				}

				attackDelay += dt;
				if (attackDelay > 2.f)
				{
					GetComponent<Sprite>()->PlayAnimation(3);
					SetStateOn(EnemyStates::ATTACK);
					attackDelay = 0.f;
				}
			}
			else if (IsStateOn(EnemyStates::ATTACK) == true)
			{
				if (GetComponent<Sprite>()->IsAnimationDone() == true)
				{
					numOfAttack++;
					if (numOfAttack < 10)
					{
						float dirY = 0;
						switch ((rand() % (3 + 1)) + 1)
						{
						case 1:
							dirY = 100.f;
							break;
						case 2:
							dirY = 0.f;
							break;
						case 3:
							dirY = -100.f;
							break;
						}

						Engine::GetObjectManager().AddObject<PEnemyBullet>(position, glm::vec3{ 8.f,8.f,0.f }, "EBullet");
						if (IsStateOn(EnemyStates::DIRECTION) == true)
						{
							Engine::GetObjectManager().GetLastObject()->SetYSpeed(dirY);
							Engine::GetObjectManager().GetLastObject()->SetXSpeed(1000.f);
						}
						else
						{
							Engine::GetObjectManager().GetLastObject()->SetYSpeed(dirY);
							Engine::GetObjectManager().GetLastObject()->SetXSpeed(-1000.f);
						}
						GetComponent<Sprite>()->PlayAnimation(3);
					}
					else
					{
						numOfAttack = 0;
						SetStateOff(EnemyStates::ATTACK);
						GetComponent<Sprite>()->PlayAnimation(0);
					}
				}
			}
		}
		else
		{
			GetComponent<Physics2D>()->Gravity(dt);
			if (GetComponent<Physics2D>()->GetVelocity().y > -0.9f &&
				GetComponent<Physics2D>()->GetVelocity().y < 0.0f)
			{
				SetStateOn(EnemyStates::ONGROUND);
			}
		}
	}
}

void PEnemy::UpdateEnemyAirShip(float dt)
{
	if (hp < 0.f)
	{
		if (IsStateOn(EnemyStates::DEATH) == false)
		{
			isInvincible = true;
			SetStateOn(EnemyStates::DEATH);
		}
		else
		{
			attackDelay += dt;
			if (attackDelay > 0.1f)
			{
				attackDelay = 0.f;
				Engine::GetParticleManager().AddSingleParticle({ Object::position.x + 48.f, Object::position.y + 48.f, 0.f }, { 96.f,96.f,0.f },
					{ 0.f,0.f,0.f }, 0.f, 10.f, { 1.f,1.f,1.f,1.f }, ParticleType::ANIMESPRI, "../Game/assets/PlatformDemo/explosion.spt");
			}

			if (Object::position.y <= -328.f)
			{
				Engine::GetParticleManager().AddSingleParticle(Object::position, { 128.f,128.f,0.f },
					{ 0.f,0.f,0.f }, 0.f, 10.f, { 1.f,1.f,1.f,1.f }, ParticleType::ANIMESPRI, "../Game/assets/PlatformDemo/explosion.spt");
				Engine::GetObjectManager().Destroy(Object::id);
			}
		}
	}

	Object::Update(dt);
	if (IsStateOn(EnemyStates::DEATH) == false)
	{
		GetComponent<Physics2D>()->SetVelocityX(-5.f);
		Hit(dt);
		if (IsStateOn(EnemyStates::ATTACK) == false)
		{
			attackDelay += 1.f * dt;
			if (attackDelay > 1.5f)
			{
				//Engine::GetObjectManager()->AddObject<PEnemy>(Object::position, glm::vec3{ 64.f, 96.f,0.f }, "Enemy", EnemyType::NORMAL);

				Engine::GetObjectManager().AddObject<PEnemy>(Object::position, glm::vec3{ 224.f, 256.f,0.f }, "Enemy", EnemyType::BIG);
				SetStateOn(EnemyStates::ATTACK);
				attackDelay = 0.f;
			}
		}
		else
		{
			GetComponent<Physics2D>()->AddForceY(7.5f);
		}
	}
	else
	{
		GetComponent<Physics2D>()->SetVelocityX(-2.5f);
		GetComponent<Physics2D>()->AddForceY(-15.f);
	}
}
