#pragma once
#include "ComponentTypes.hpp"

class Object;
class Component
{
public:
	friend class Object;

	Component(ComponentTypes type) : componentType(type) {};
	virtual ~Component() { };

	virtual void Init() = 0;
	virtual void Update(float dt) = 0;
	virtual void End() = 0;

	Object * GetOwner() const { return owner; }
	void SetOwner(Object* owner_) { this->owner = owner_; }
	ComponentTypes GetType() { return componentType; }

private:
	Object* owner = nullptr;
	ComponentTypes componentType;
};