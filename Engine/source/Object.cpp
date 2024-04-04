//Author: DOYEONG LEE
//Project: CubeEngine
//File: Object.cpp
#include "Object.hpp"

#include <algorithm>
#include <iostream>

Object::Object(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
{
	position = pos_;
	size = size_;

	this->objectType = objectType;

	objectName = name;
}

Object::Object(const Object& rhs) : position(rhs.position), speed(rhs.speed), size(rhs.size), 
objectType(rhs.objectType), objectName(rhs.objectName), angle(rhs.angle), componentList(rhs.componentList)
{
	for (auto& c : componentList)
	{
		c->SetOwner(this);
	}
}

void Object::Init()
{
}

void Object::Update(float dt)
{
	//for (auto comp : componentList)
	//{
	//	comp->Update(dt);
	//}
	for (auto& comp : componentList) 
	{
		comp->Update(dt);
	}
	Draw(dt);
}

void Object::Draw(float /*dt*/)
{
}

void Object::SetRotate(float value)
{
	{
		if (value > 360.f)
		{
			angle = value - 360.f;

		}
		else if (value < 0.f)
		{
			angle = 360.f + value;
		}
		else
		{
			angle = value;
		}
	}
}

void Object::DestroyAllComponents()
{
	for (auto* comp : componentList)
	{
		delete comp;
	}
	componentList.clear();
}
