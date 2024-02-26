//Author: DOYEONG LEE
//Project: CubeEngine
//File: PPlayer.hpp
#pragma once
#include "Object.hpp"

enum class PlayerStates
{
	DIRECTION = 1, // off : LEFT, on : RIGHT
	MOVE = 2,
	ATTACK = 4,
	JUMPING = 8,
	FALLING = 16,
	ONGROUND = 32,
};

class PlatformDemoSystem;
class PPlayer : public Object
{
public:
	PPlayer() = default;
	PPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType);
	PPlayer(glm::vec3 pos_, glm::vec3 size_, std::string name, PlatformDemoSystem* sys);
	~PPlayer() { End(); }

	void Init() override;
	void Update(float dt) override;
	//void Draw(float dt) override;
	void End();
	void CollideObject(Object* obj) override;

	void SetInvincibleState(bool state_) { isInvincible = state_; }
	bool GetInvincibleState() { return isInvincible; }

	void SetStateOn(PlayerStates state_)
	{
		if (IsStateOn(state_) == false)
			state = state | static_cast<const int>(state_);
	}
	void SetStateOff(PlayerStates state_)
	{
		if (IsStateOn(state_) == true)
			state = state ^ static_cast<const int>(state_);
	}
	bool IsStateOn(PlayerStates state_) { return state & static_cast<const int>(state_); }
private:
	void Control (float dt);
	void Jumping();

	bool isInvincible = false;
	bool canAttack = true;

	float attackDelay = 0.f;
	float invincibleDelay = 0.f;

	int    state = 1;

	PlatformDemoSystem* platformDemoSystem = nullptr;
};