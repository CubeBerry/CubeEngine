//Author: DOYEONG LEE
//Project: CubeEngine
//File: PBullet.hpp
#pragma once
#include "Object.hpp"

class PBullet : public Object
{
public:
    PBullet() = default;
	PBullet(glm::vec3 pos_, glm::vec3 size_, std::string name);
	~PBullet() {};

	void Init() override;
	void Update(float dt) override;

	void SetDamage(float amount) { damage = amount;}
	float GetDamage() {return damage;}
	
	void MakeHitParticle();
protected:
	float damage = 0.5f;

	glm::vec2 prePos { 0.f, 0.f };
};