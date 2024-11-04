//Author: DOYEONG LEE
//Project: CubeEngine
//File: BeatEmUpDemoSystem.cpp
#include "BeatEmUpDemo/BeatEmUpDemoSystem.hpp"
#include "BeatEmUpDemo/BEUEnemy.hpp"
#include "Engine.hpp"

#include <iostream>
#include <fstream>

#include "imgui.h"

void BeatEmUpDemoSystem::Init()
{
	maxHp = 30.f;
	hp = 30.f;
	
	healthBar = new Sprite();
	healthBar->AddMeshWithTexture("hpbar", {0.f,1.f,0.f,1.f});
	healthBar->SetSpriteDrawType(SpriteDrawType::UI);

	emeyHealthBar = new Sprite();
	emeyHealthBar->AddMeshWithTexture("hpbar", { 0.f,0.f,0.f,0.f });
	emeyHealthBar->SetSpriteDrawType(SpriteDrawType::UI);
}

void BeatEmUpDemoSystem::Update(float dt)
{
	glm::vec2 viewSize = Engine::GetCameraManager().GetViewSize();
	glm::vec2 center = Engine::GetCameraManager().GetCenter();
	healthBar->UpdateModel({ (-viewSize.x / 2.f + 320.f) + center.x + 32.f - (320.f - (320.f * (1.f / maxHp * hp)) / 2.f) , (viewSize.y / 2.f - 64.f) + center.y, 0.f }, { 320.f * (1.f / maxHp * hp), 32.f, 0.f }, 0.f);
	healthBar->UpdateProjection();
	healthBar->UpdateView();

	emeyHealthBar->UpdateModel({ (-viewSize.x / 2.f + 320.f) + center.x + 32.f - (320.f - (320.f * (1.f / eMaxHp * eHp)) / 2.f) , (viewSize.y / 2.f - 108.f) + center.y, 0.f }, { 320.f * (1.f / eMaxHp * eHp), 32.f, 0.f }, 0.f);
	emeyHealthBar->UpdateProjection();
	emeyHealthBar->UpdateView();

	if (isEnemyHit == true)
	{
		enemyHpBarShowDelay += dt;
		if (enemyHpBarShowDelay > 4.5f)
		{
			isEnemyHit = false;
			enemyHpBarShowDelay = 0.f;
			emeyHealthBar->SetColor({ 0.f,0.f,0.f,0.f });
		}
	}
}

void BeatEmUpDemoSystem::End()
{
	delete healthBar;
	delete emeyHealthBar;
}

void BeatEmUpDemoSystem::ShowEnemyHpBarOn(float hp_, float maxHp_)
{
	isEnemyHit = true;
	enemyHpBarShowDelay = 0.f;

	eMaxHp = maxHp_;
	eHp = hp_;
	emeyHealthBar->SetColor({ 1.f,0.f,0.f,1.f });
}

void BeatEmUpDemoSystem::SpawnEnemy(glm::vec3 pos_, glm::vec3 size_, std::string name)
{
	Engine::GetObjectManager().AddObject<BEUEnemy>(pos_, size_, name, this);
	enemyList.push_back(reinterpret_cast<BEUEnemy*>(Engine::GetObjectManager().GetLastObject()));
}

void BeatEmUpDemoSystem::DeleteEnemy(BEUEnemy* enemy)
{
	auto iterator = std::find(enemyList.begin(), enemyList.end(), enemy);
	enemyList.erase(iterator);
}
