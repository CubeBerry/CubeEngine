//Author: DOYEONG LEE
//Project: CubeEngine
//File: Object.hpp
#pragma once
#include <string>
#include <vector>

#include "ObjectType.hpp"
#include "glm/glm.hpp"
#include "Component.hpp"

class Object
{
public:
	Object() = default;
	Object(glm::vec3 pos_, glm::vec3 size_, std::string name = "", ObjectType objectType = ObjectType::NONE);
	Object(const Object& rhs);
	~Object() { DestroyAllComponents(); };
	virtual void Init();
	virtual void Update(float dt);
	virtual void Draw(float dt);
	virtual void CollideObject(Object* /*obj*/) {};

	int        GetId() { return id; }
	glm::vec3     GetPosition() { return position; }
	glm::vec3     GetSize() { return size; }
	glm::vec3     GetSpeed() { return speed; }

	float GetRotate() { return angle; }

	ObjectType GetObjectType() { return objectType; }
	std::string GetName() { return objectName; }
	
	void       SetId(int value) { id = value; }
	void       SetRotate(float value);

	void       SetXPosition(float newX) { position.x = newX; }
	void       SetYPosition(float newY) { position.y = newY; }
	void       SetZPosition(float newZ) { position.z = newZ; }
	void       SetXSpeed(float newX) { speed.x = newX; }
	void       SetYSpeed(float newY) { speed.y = newY; }
	void       SetZSpeed(float newZ) { speed.z = newZ; }
	void       SetXSize(float newX) { size.x = newX; }
	void       SetYSize(float newY) { size.y = newY; }
	void       SetZSize(float newZ) { size.z = newZ; }

	void       SetName(std::string name) { objectName = name; }
	void       SetObjectType(ObjectType type) { objectType = type; }

	template <typename ComponentTypes> bool HasComponent()
	{
		for (auto list : componentList)
		{
			if (typeid(*list).name() == typeid(ComponentTypes).name())
				return true;
		}
		return false;
	}

	template<typename ComponentTypes> constexpr void AddComponent()
	{
		if (HasComponent<ComponentTypes>())
		{
			return;
		}
		ComponentTypes* componentType = new ComponentTypes();
		dynamic_cast<Component*>(componentType)->SetOwner(this);
		this->componentList.push_back(std::move(componentType));
	}

	template<typename ComponentTypes> ComponentTypes* GetComponent()
	{
		for (auto list : componentList)
		{
			if (typeid(*list).name() == typeid(ComponentTypes).name())
				return dynamic_cast<ComponentTypes*>(list);
		}
		return nullptr;
	}

	//template <typename T> bool HasComponent()
	//{
	//	for (auto list : componentList)
	//	{
	//		if (typeid(*list).name() == typeid(T).name())
	//			return true;
	//	}
	//	return false;
	//}

	//template<typename T> constexpr void AddComponent()
	//{
	//	if (HasComponent<T>())
	//	{
	//		return;
	//	}/*
	//	ComponentTypes* componentType = new ComponentTypes();
	//	dynamic_cast<Component*>(componentType)->SetOwner(this);
	//	this->componentList.push_back(std::move(componentType));*/
	//	this->componentList.push_back(std::move(new T()));
	//	this->componentList.at(this->componentList.size() - 1)->SetOwner(this);
	//}

	//template<typename T> T* GetComponent()
	//{
	//	for (auto list : componentList)
	//	{
	//		if (typeid(*list).name() == typeid(T).name())
	//			return dynamic_cast<T*>(list);
	//	}
	//	return nullptr;
	//}

protected:
	void DestroyAllComponents();

	glm::vec3 position{ 0.f, 0.f, 0.f };
	glm::vec3 speed{ 0.f, 0.f, 0.f };
	glm::vec3 size{ 0.f, 0.f, 0.f };
	float angle = 0.f;

	int        id = 0;
	ObjectType objectType = ObjectType::NONE;

	std::string objectName = "";
	bool isDrawAble = true;
	std::vector<Component*> componentList;
};