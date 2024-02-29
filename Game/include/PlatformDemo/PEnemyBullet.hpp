//Author: DOYEONG LEE
//Project: CubeEngine
//File: PEnemyBullet.hpp
#pragma once
#include "Object.hpp"

class PEnemyBullet : public Object
{
public:
	PEnemyBullet() = default;
	PEnemyBullet(glm::vec3 pos_, glm::vec3 size_, std::string name);
	~PEnemyBullet() {};

	void Init() override;
	void Update(float dt) override;

	void SetDamage(float amount) { damage = amount;}
	float GetDamage() {return damage;}

protected:
	float damage = 5.f;
	float delay = 0.f;
};