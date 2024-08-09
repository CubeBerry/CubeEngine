//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUEnemyAttackBox.hpp
#pragma once
#include "Object.hpp"

class BEUEnemyAttackBox : public Object
{
public:
	BEUEnemyAttackBox() = default;
	BEUEnemyAttackBox(glm::vec3 pos_, glm::vec3 size_, std::string name, Object* parent_);
	~BEUEnemyAttackBox() { parent = nullptr; }

	void Init() override;
	void Update(float dt) override;

	void SetDamage(float amount) { damage = amount;}
	float GetDamage() {return damage;}

protected:
	float damage = 5.f;
	float delay = 0.f;

	Object* parent = nullptr;
};