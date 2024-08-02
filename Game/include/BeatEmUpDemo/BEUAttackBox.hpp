//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUAttackBox.hpp
#pragma once
#include "Object.hpp"

class BEUAttackBox : public Object
{
public:
	BEUAttackBox() = default;
	BEUAttackBox(glm::vec3 pos_, glm::vec3 size_, std::string name, Object* parent_);
	~BEUAttackBox() { parent = nullptr; }

	void Init() override;
	void Update(float dt) override;

	void SetDamage(float amount) { damage = amount;}
	float GetDamage() {return damage;}
	
	void MakeHitParticle();
protected:
	float damage = 0.5f;

	Object* parent = nullptr;
};