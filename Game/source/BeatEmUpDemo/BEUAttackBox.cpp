//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUAttackBox.cpp

#include "BeatEmUpDemo/BEUAttackBox.hpp"
#include "BeatEmUpDemo/BEUObject.hpp"

#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "Engine.hpp"

BEUAttackBox::BEUAttackBox(glm::vec3 offset_, glm::vec3 size_, std::string name, Object* parent_, float lifeTime_, AttackBoxType type_, ObjectType objType)
	: Object(offset_, size_, name, objType)
{
	Init();
	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->AddCollidePolygonAABB(size_ / 2.f);
	GetComponent<Physics2D>()->SetGhostCollision(true);

	AddComponent<Sprite>();
	GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,1.f });
	parent = parent_;
	lifeTime = lifeTime_;
	offset = offset_;
	type = type_;

	position = glm::vec3{ parent->GetPosition() + offset };
}

void BEUAttackBox::Init()
{
}

void BEUAttackBox::Update(float dt)
{
	position = glm::vec3{ parent->GetPosition() + offset };
	Object::Update(dt);

	delay += dt;
	if (lifeTime < delay)
	{
		if (isDeleted == false)
		{
			isDeleted = true;
			Engine::GetObjectManager().Destroy(GetId());
		}
	}
}

void BEUAttackBox::CollideObject(Object* obj)
{
	if (objectType == ObjectType::BULLET)
	{
		switch (obj->GetObjectType())
		{
		case ObjectType::ENEMY:
			if (GetComponent<Physics2D>()->CheckCollision(obj) == true && (position.z < obj->GetPosition().z + 1.f && position.z > obj->GetPosition().z - 1.f))
			{
				static_cast<BEUObject*>(parent)->SetIsAttackHit(true);
				if (isDeleted == false)
				{
					isDeleted = true;
					Engine::GetObjectManager().Destroy(GetId());
				}
			}
			break;
		}
	}
	else if (objectType == ObjectType::ENEMYBULLET)
	{
	}
}

void BEUAttackBox::MakeHitParticle()
{
	////rand() % (마지막 값 - 시작 값 + 1) + 시작 값
	//int amount = rand() % (10 - 5 + 1) + 5;
	//for (int i = 0; i < amount; i++)
	//{
	//	float colorG = static_cast<float>(rand() % (9 - 3 + 1) + 3) * 0.1f;
	//	float pSize = static_cast<float>(rand() % (20 - 10 + 1) + 10) * 0.1f;

	//	glm::vec2 newVel = { static_cast<float>(rand() % (2 - (-2) + 1) + (-2)) , -(static_cast<float>(rand() % ((10) - (5) + 1) + (5))) };
	//	float lifeTime = static_cast<float>(rand() % (16 - 8 + 1) + 8);
	//	//ParticleSystem::Instance().AddSingleParticle(position, { pSize, pSize }, newVel, 0.f, lifeTime, { 1.f, colorG, 0.f,1.f }, ParticleType::REC, "", 0.5f);
	//}
}
