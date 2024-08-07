//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUEnemy.hpp
#pragma once
#include "Object.hpp"
#include "BeatEmUpDemo/BEUObjectState.hpp"

class BeatEmUpDemoSystem;
class BEUEnemy : public Object
{
public:
	BEUEnemy() = default;
	BEUEnemy(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType);
	BEUEnemy(glm::vec3 pos_, glm::vec3 size_, std::string name, BeatEmUpDemoSystem* sys);
	~BEUEnemy() { End(); }

	void Init() override;
	void Update(float dt) override;
	//void Draw(float dt) override;
	void End();
	void CollideObject(Object* obj) override;

	void SetInvincibleState(bool state_) { isInvincible = state_; }
	bool GetInvincibleState() { return isInvincible; }

	void SetStateOn(BEUObjectStates state_)
	{
		if (IsStateOn(state_) == false)
			state = state | static_cast<const int>(state_);
	}
	void SetStateOff(BEUObjectStates state_)
	{
		if (IsStateOn(state_) == true)
			state = state ^ static_cast<const int>(state_);
	}
	bool IsStateOn(BEUObjectStates state_) { return state & static_cast<const int>(state_); }

	void SetIsAttackHit(bool state_);
private:
	void Moving(float dt);
	void Attack(float dt);
	void Jumping();
	void KnockBack(float dt);

	bool isInvincible = false;
	bool canAttack = false;

	float attackDelay = 0.f;
	float layingDelay = 0.f;
	float hitDelay = 0.f;
	float invincibleDelay = 0.f;

	int state = 1;
	int combo = 0;

	BeatEmUpDemoSystem* beatEmUpDemoSystem = nullptr;
	bool isAttackHit = false;
};