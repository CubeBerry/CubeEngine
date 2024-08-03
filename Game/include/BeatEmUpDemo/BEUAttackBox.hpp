//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUAttackBox.hpp
#pragma once
#include "Object.hpp"

enum class AttackBoxType
{
	NORMAL,
	FINISH
};

class BEUAttackBox : public Object
{
public:
	BEUAttackBox() = default;
	BEUAttackBox(glm::vec3 offset_, glm::vec3 size_, std::string name, Object* parent_, float lifeTime_, AttackBoxType type_ = AttackBoxType::NORMAL, ObjectType objType = ObjectType::BULLET);
	~BEUAttackBox() { parent = nullptr; }

	void Init() override;
	void Update(float dt) override;
	void CollideObject(Object* obj) override;

	void SetDamage(float amount) { damage = amount;}
	float GetDamage() {return damage;}
	AttackBoxType GetAttackBoxType() { return type; }

	void MakeHitParticle();
protected:
	float damage = 0.5f;

	float delay = 0.f;
	float lifeTime = 0.f;
	bool isDeleted = false;

	Object* parent = nullptr;
	glm::vec3 offset{ 0.f,0.f,0.f };
	AttackBoxType type = AttackBoxType::NORMAL;
};