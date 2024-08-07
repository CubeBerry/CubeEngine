//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUPlayer.hpp
#pragma once
#include "Object.hpp"
#include "BeatEmUpDemo/BEUObjectState.hpp"

class BeatEmUpDemoSystem;
class BEUPlayer : public Object
{
public:
	BEUPlayer() = default;
	BEUPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType);
	BEUPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name, BeatEmUpDemoSystem* sys);
	~BEUPlayer() { End(); }

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

	//player
	void SetIsAttackHit(bool state_);
	//player
private:
	void Jumping();
	void KnockBack(float dt);

	bool isInvincible = false;
	bool canAttack = true;

	float attackDelay = 0.f;
	float layingDelay = 0.f;
	float hitDelay = 0.f;
	float invincibleDelay = 0.f;

	int state = 1;
	int combo = 0;

	//player
	BeatEmUpDemoSystem* beatEmUpDemoSystem = nullptr;
	bool isAttackHit = false;
	void Control (float dt);
	//player
};