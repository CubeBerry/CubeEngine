//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUEnemyAttackBox.cpp
#include "BeatEmUpDemo/BEUEnemyAttackBox.hpp"

#include "BasicComponents/Physics2D.hpp"
#include "Engine.hpp"
#include <iostream>

BEUEnemyAttackBox::BEUEnemyAttackBox(glm::vec3 pos_, glm::vec3 size_, std::string name, Object* parent_)
	: Object(pos_, size_, name, ObjectType::ENEMYBULLET)
{
	Init();
	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->AddCollidePolygonAABB(size_ / 2.f);
	GetComponent<Physics2D>()->SetGhostCollision(true);

	AddComponent<Sprite>();
	GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,1.f });
	parent = parent_;
}

void BEUEnemyAttackBox::Init()
{
}

void BEUEnemyAttackBox::Update(float dt)
{
	Object::Update(dt);

	position.x += speed.x * dt;
	position.y += speed.y * dt;

	if (Engine::GetCameraManager().IsInCamera(this) == false)
	{
		Engine::GetObjectManager().Destroy(Object::id);
	}
}
