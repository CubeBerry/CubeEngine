//Author: DOYEONG LEE
//Project: CubeEngine
//File: PEnemy.hpp
#pragma once
#include "Object.hpp"

enum class EnemyType
{
	NORMAL,
	BIG,
	AIRSHIP,
	NONE
};

enum class EnemyStates
{
	DIRECTION = 1, // off : LEFT, on : RIGHT
	MOVE = 2,
	ATTACK = 4,
	JUMPING = 8,
	FALLING = 16,
	ONGROUND = 32,
	DEATH = 64,
	TARGETFOUND = 128,
};

class PEnemy : public Object
{
public:
    PEnemy() = default;
	PEnemy(glm::vec3 pos_, glm::vec3 size_, std::string name, EnemyType type);
	~PEnemy() {};

	void Init() override;
	void Update(float dt) override;
	void CollideObject(Object* obj) override;

	void SetHp(float amount) { hp = amount;}
	void SetMaxHp(float amount) { MaxHp = amount; }
	float GetHp() { return hp; }
	float GetMaxHp() { return MaxHp; }

	void SetIsHit(bool state_) { isHit = state_; }
	void SetInvincibleState(bool state_) { isInvincible = state_;}
	bool GetInvincibleState() { return isInvincible; }

	glm::vec2 GetSpawnPosition() { return spawnPosition; }
	void SetSpawnPosition(glm::vec2 pos) { spawnPosition = pos; }
	void SetAngleToTarget(Object* target);

	void MoveTowardAngle(float dt);

	void Hit(float dt);

	void SetStateOn(EnemyStates state_)
	{
		if (IsStateOn(state_) == false)
			state = state | static_cast<const int>(state_);
	}
	void SetStateOff(EnemyStates state_)
	{
		if (IsStateOn(state_) == true)
			state = state ^ static_cast<const int>(state_);
	}
	bool IsStateOn(EnemyStates state_) { return state & static_cast<const int>(state_); }
protected:
	void UpdateEnemyNormal(float dt);
	void UpdateEnemyBig(float dt);
	void UpdateEnemyAirShip(float dt);

	float hp = 1;
	float MaxHp = 0;

	float maxInvincibleDelay = 0.1f;
	float invincibleDelay = 0.f;

	float attackDelay = 0.f;
	int numOfAttack = 0;

	bool isHit = false;
	bool isInvincible = false;
	int    state = 1;

	glm::vec2 spawnPosition = { 0.f,0.f };
	EnemyType eType = EnemyType::NONE;
};