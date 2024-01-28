//Author: DOYEONG LEE
//Project: CubeEngine
//File: Object.cpp
#include "Object.hpp"

#include <iostream>

Object::Object(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
{
	position = pos_;
	size = size_;

	collisionBoxSize.x = size.x;
	collisionBoxSize.y = size.y;

	this->objectType = objectType;

	objectName = name;
	SetMinMax();
}

Object::Object(const Object& rhs) : position(rhs.position), speed(rhs.speed), size(rhs.size),
drawType(rhs.drawType), objectType(rhs.objectType), objectName(rhs.objectName), spriteName(rhs.spriteName),
angle(rhs.angle), min(rhs.min), max(rhs.max), collisionBoxSize(rhs.collisionBoxSize), collisionBoxPosition(rhs.collisionBoxPosition), componentList(rhs.componentList)
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
	for (auto comp : componentList)
	{
		comp->Update(dt);
	}
	Draw(dt);
	SetMinMax();
}

void Object::Draw(float /*dt*/)
{
	//if (HasComponent<MaterialComponent>())
	//{
	//	switch (drawType)
	//	{
	//	case DrawType::RECTANGLE:
	//		break;
	//	case DrawType::RECTANGLELINE:
	//		break;
	//	case DrawType::SPRITE:
	//		break;
	//	case DrawType::SPRITEANIMATION:
	//		break;
	//	}
	//}
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

void Object::SetSpriteName(std::string name)
{
	spriteName = name;
}

void Object::SetMinMax()
{
	min.x = position.x + collisionBoxPosition.x - collisionBoxSize.x;
	max.x = position.x + collisionBoxPosition.x + collisionBoxSize.x;
	min.y = position.y + collisionBoxPosition.y - collisionBoxSize.y;
	max.y = position.y + collisionBoxPosition.y + collisionBoxSize.y;
	min.z = position.z + collisionBoxPosition.z - collisionBoxSize.z;
	max.z = position.z + collisionBoxPosition.z + collisionBoxSize.z;
}

bool Object::isCollideWithObjectIn2D(Object& object) noexcept
{
	if (&object != nullptr)
	{
		return !(max.x < object.GetMin().x || object.GetMax().x < min.x || max.y < object.GetMin().y ||
			object.GetMax().y < min.y);
	}
	return false;
}

bool Object::isCollideWithPointIn2D(glm::vec3 point) noexcept
{
	return !(max.x < point.x || point.x < min.x || max.y < point.y ||
		point.y < min.y);
}

void Object::DestroyAllComponents()
{
	for (auto comp : componentList)
	{
		delete comp;
	}
}
