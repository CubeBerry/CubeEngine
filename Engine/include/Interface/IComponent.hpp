//Author: DOYEONG LEE
//Project: CubeEngine
//File: IComponent.hpp
#pragma once
#include "ComponentTypes.hpp"

class Object;
class IComponent
{
public:
	friend class Object;

	IComponent(ComponentTypes type) : componentType(type) {};
	virtual ~IComponent() { };

	virtual void Init() = 0;
	virtual void Update(float dt) = 0;
	virtual void End() = 0;

	Object* GetOwner() const { return owner; }
	void SetOwner(Object* owner_) { this->owner = owner_; }
	ComponentTypes GetType() const { return componentType; }

private:
	Object* owner = nullptr;
	ComponentTypes componentType;
};