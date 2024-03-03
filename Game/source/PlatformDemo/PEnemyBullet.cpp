//Author: DOYEONG LEE
//Project: CubeEngine
//File: PEnemyBullet.cpp
#include "PlatformDemo/PEnemyBullet.hpp"

#include "BasicComponents/Physics2D.hpp"
#include "Engine.hpp"
#include <iostream>

PEnemyBullet::PEnemyBullet(glm::vec3 pos_, glm::vec3 size_, std::string name)
	: Object(pos_, size_, name, ObjectType::ENEMYBULLET)
{
	Init();
	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->AddCollidePolygonAABB(size_ / 2.f);
	GetComponent<Physics2D>()->SetGhostCollision(true);

	AddComponent<Sprite>();
	GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,1.f });
}

void PEnemyBullet::Init()
{
}

void PEnemyBullet::Update(float dt)
{
	Object::Update(dt);

	position.x += speed.x * dt;
	position.y += speed.y * dt;

	if (Engine::GetCameraManager().IsInCamera(this) == false)
	{
		Engine::GetObjectManager().Destroy(Object::id);
	}
}
