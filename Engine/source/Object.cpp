//Author: DOYEONG LEE
//Project: CubeEngine
//File: Object.cpp
#include "Engine.hpp"
#include "Object.hpp"

#include <iostream>

Object::Object()
{
}

Object::Object(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
{
	position = pos_;
	size = size_;

	this->objectType = objectType;

	objectName = name;
	Engine::GetLogger().LogDebug(LogCategory::Object, "Object Added : " + objectName);
}

Object::Object(const Object& rhs) : position(rhs.position), speed(rhs.speed), size(rhs.size), 
objectType(rhs.objectType), objectName(rhs.objectName), angle(rhs.angle), componentList(rhs.componentList)
{
	for (auto& c : componentList)
	{
		c->SetOwner(this);
	}
}

Object::~Object()
{
	DestroyAllComponents();
	Engine::GetLogger().LogDebug(LogCategory::Object, "Object Deleted : " + objectName);
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
			angle.z = value - 360.f;

		}
		else if (value < 0.f)
		{
			angle.z = 360.f + value;
		}
		else
		{
			angle.z = value;
		}
	}
}

void Object::SetRotate(glm::vec3 value)
{
	SetXRotate(value.x);
	SetYRotate(value.y);
	SetZRotate(value.z);
}

void Object::SetXRotate(float value)
{
	{
		if (value > 360.f)
		{
			angle.x = value - 360.f;

		}
		else if (value < 0.f)
		{
			angle.x = 360.f + value;
		}
		else
		{
			angle.x = value;
		}
	}
}

void Object::SetYRotate(float value)
{
	{
		if (value > 360.f)
		{
			angle.y = value - 360.f;

		}
		else if (value < 0.f)
		{
			angle.y = 360.f + value;
		}
		else
		{
			angle.y = value;
		}
	}
}

void Object::SetZRotate(float value)
{
	{
		if (value > 360.f)
		{
			angle.z = value - 360.f;

		}
		else if (value < 0.f)
		{
			angle.z = 360.f + value;
		}
		else
		{
			angle.z = value;
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
