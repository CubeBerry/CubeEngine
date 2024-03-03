//Author: DOYEONG LEE
//Project: CubeEngine
//File: PBullet.cpp
#include "PlatformDemo/PBullet.hpp"
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "Engine.hpp"

PBullet::PBullet(glm::vec3 pos_, glm::vec3 size_, std::string name)
	: Object(pos_, size_, name, ObjectType::BULLET)
{
	Init();
	AddComponent<Physics2D>();
	GetComponent<Physics2D>()->AddCollidePolygonAABB(size_ / 2.f);
	GetComponent<Physics2D>()->SetGhostCollision(true);

	AddComponent<Sprite>();
	GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,1.f });
}

void PBullet::Init()
{
}

void PBullet::Update(float dt)
{
	Object::Update(dt);

	position.x += speed.x * dt;
	position.y += speed.y * dt;

	if (Engine::GetCameraManager().IsInCamera(this) == false)
	{
		Engine::GetObjectManager().Destroy(Object::id);
	}
}

void PBullet::MakeHitParticle()
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
