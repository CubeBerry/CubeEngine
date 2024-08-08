//Author: DOYEONG LEE
//Project: CubeEngine
//File: BeatEmUpDemoSystem.hpp
#pragma once

#include "glm/vec3.hpp"
#include <vector>
#include <string>

class BEUEnemy;
class Sprite;
class BeatEmUpDemoSystem
{
public:
	BeatEmUpDemoSystem() {}
	~BeatEmUpDemoSystem() = default;

	void Init();
	void Update(float dt);
	void End();

	void SetHp(float amount) { hp = amount; }
	void SetMaxHp(float amount) { maxHp = amount; }

	float GetHp() { return hp; }
	float GetMaxHp() { return maxHp; }

	void HpDecrease(float damage) { hp -= damage; }
	void ShowEnemyHpBarOn(float hp_, float maxHp_);

	void SetIsCameraMoveAble(bool state) { isCameraMoveAble = state; }
	bool GetIsCameraMoveAble() { return isCameraMoveAble; }

	void SpawnEnemy(glm::vec3 pos_, glm::vec3 size_, std::string name);
	std::vector<BEUEnemy*> GetEnemyList() { return enemyList;}
	void DeleteEnemy(BEUEnemy* enemy);
#ifdef _DEBUG
#endif
protected:
	Sprite* healthBar = nullptr;
	Sprite* emeyHealthBar = nullptr;

	bool isCameraMoveAble = true;
	bool isEnemyHit = false;
	float enemyHpBarShowDelay = 0.f;

	float maxHp = 30.f;
	float hp = 30.f;

	float eMaxHp = 0.f;
	float eHp = 0.f;

	std::vector<BEUEnemy*> enemyList;
};