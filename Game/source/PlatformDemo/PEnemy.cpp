//Author: DOYEONG LEE
//Project: CubeEngine
//File: PEnemy.cpp
#include "PlatformDemo/PEnemy.hpp"
#include "PlatformDemo/PEnemyBullet.hpp"
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "Engine.hpp"

PEnemy::PEnemy(glm::vec3 pos_, glm::vec3 size_, std::string name)
	: Object(pos_, size_, name, ObjectType::ENEMY)
{
	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->SetMinVelocity({ 0.01f, 0.1f });
	GetComponent<Physics2D>()->SetGravity(40.f);
	GetComponent<Physics2D>()->SetFriction(0.9f);
	GetComponent<Physics2D>()->SetMaxVelocity({ 10.f,20.f });
	GetComponent<Physics2D>()->AddCollidePolygonAABB(size_ / 2.f);
	GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);

	AddComponent<Sprite>();
	GetComponent<Sprite>()->AddQuad({ 1.f,0.f,0.f,1.f });
}

void PEnemy::Init()
{

}

void PEnemy::Update(float dt)
{
	Hit(dt);
	Object::Update(dt);

	if (Engine::GetCameraManager()->IsInCamera(this) == true)
	{
		GetComponent<Physics2D>()->Gravity(dt);
	}

	if (hp < 0.f)
	{
		Engine::GetObjectManager()->Destroy(Object::id);
	}

	attackDelay += dt;
	if (attackDelay > 2.f)
	{
		Engine::GetObjectManager()->AddObject<PEnemyBullet>(position, glm::vec3{ 8.f,8.f,0.f }, "EBullet");
		if (Engine::GetObjectManager()->FindObjectWithName("Player")->GetPosition().x > Object::GetPosition().x)
		{
			Engine::GetObjectManager()->GetLastObject()->SetXSpeed(1000.f);
		}
		else
		{
			Engine::GetObjectManager()->GetLastObject()->SetXSpeed(-1000.f);
		}
		attackDelay = 0.f;
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
			GetComponent<Sprite>()->SetColor({ 1.f,0.f,0.f,1.f });
			invincibleDelay = 0.f;
			isHit = false;
			isInvincible = false;
		}
	}
}
