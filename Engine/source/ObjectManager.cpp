//Author: DOYEONG LEE
//Project: CubeEngine
//File: ObjectManager.cpp
#include "ObjectManager.hpp"

#include <iostream>
void ObjectManager::Update(float dt)
{
	std::for_each(objectMap.begin(), objectMap.end(), [&](auto& obj) { obj.second->Update(dt); });
	std::for_each(objectsToBeDeleted.begin(), objectsToBeDeleted.end(), [&](int id) { objectMap.erase(id); });
	objectsToBeDeleted.clear();
}

void ObjectManager::End()
{
	DestroyAllObjects();
}

void ObjectManager::Draw(float dt)
{
	std::for_each(objectMap.begin(), objectMap.end(), [&](auto& obj) { obj.second->Draw(dt); });
}

void ObjectManager::Destroy(int id)
{
	objectsToBeDeleted.push_back(id);
}

void ObjectManager::DestroyAllObjects()
{
	lastObjectID = 0;

	std::for_each(objectsToBeDeleted.begin(), objectsToBeDeleted.end(), [&](int id) { objectMap.erase(id); });
	objectsToBeDeleted.clear();
	objectMap.clear();
}

Object* ObjectManager::FindObjectWithName(std::string name)
{
	Object* temp = nullptr;
	std::for_each(objectMap.begin(), objectMap.end(), [&](auto& obj)
	{
		if (obj.second->GetName() == name)
		{
			temp = obj.second.get();
		}
	});
	return temp;
}



