//Author: DOYEONG LEE
//Project: CubeEngine
//File: PEnemy.hpp
#pragma once
#include "Object.hpp"

class PEnemy : public Object
{
public:
    PEnemy() = default;
	PEnemy(glm::vec3 pos_, glm::vec3 size_, std::string name);
	~PEnemy() {};

	void Init() override;
	void Update(float dt) override;

	void SetHp(float amount) { hp = amount;}
	void SetMaxHp(float amount) { MaxHp = amount; }
	float GetHp() { return hp; }
	float GetMaxHp() { return MaxHp; }

	void SetIsHit(bool state) { isHit = state; }
	void SetInvincibleState(bool state) { isInvincible = state;}
	bool GetInvincibleState() { return isInvincible; }

	glm::vec2 GetSpawnPosition() { return spawnPosition; }
	void SetSpawnPosition(glm::vec2 pos) { spawnPosition = pos; }
	void SetAngleToTarget(Object* target);

	void MoveTowardAngle(float dt);

	void Hit(float dt);
protected:

	float hp = 1;
	float MaxHp = 0;

	float maxInvincibleDelay = 0.1f;
	float invincibleDelay = 0.f;

	float attackDelay = 0.f;

	bool isHit = false;
	bool isInvincible = false;

	glm::vec2 spawnPosition = { 0.f,0.f };
};