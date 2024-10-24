//Author: DOYEONG LEE
//Project: CubeEngine
//File: ObjectManager.cpp
#include "ObjectManager.hpp"

#include <iostream>


#ifdef _DEBUG
#include "Engine.hpp"
#include "BasicComponents/Physics3D.hpp"
#include "imgui.h"
#endif

void ObjectManager::Update(float dt)
{
	std::for_each(objectMap.begin(), objectMap.end(), [&](auto& obj) { obj.second->Update(dt); });
	//std::for_each(objectsToBeDeleted.begin(), objectsToBeDeleted.end(), [&](int id) { objectMap.at(id).reset(); objectMap.erase(id); });
	//objectsToBeDeleted.clear();
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
    objectMap.at(id).get()->DestroyAllComponents();

    //objectsToBeDeleted.push_back(id);
    //std::for_each(objectsToBeDeleted.begin(), objectsToBeDeleted.end(), [&](int id) { objectMap.at(id).reset(); objectMap.erase(id); });
    //objectsToBeDeleted.clear();
}

void ObjectManager::DestroyAllObjects()
{
	lastObjectID = 0;
	for (auto& obj : objectMap) 
	{
		obj.second.reset();
	}
	objectsToBeDeleted.clear();
	objectMap.clear();
}

Object* ObjectManager::FindObjectWithName(std::string name)
{
	for (auto& obj : objectMap)
	{
		if (obj.second.get()->GetName() == name)
		{
			return obj.second.get();
		}
	}
	return nullptr;
}

#ifdef _DEBUG
void ObjectManager::ObjectControllerForImGui()
{
    ImGui::Begin("ObjectController");
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, 3 * ImGui::GetTextLineHeightWithSpacing()));
    ImGui::BeginChild("Scolling");
    int index = 0;
    for (auto& object : GetObjectMap())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, (currentIndex == index) ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Text));
        if (ImGui::Selectable(object.second.get()->GetName().c_str(), index))
        {
            currentIndex = index;
        }
        ImGui::PopStyleColor();
        index++;
    }
    ImGui::EndChild();

    Object* obj = FindObjectWithId(currentIndex);

    glm::vec3 position = obj->GetPosition();
    glm::vec3 size = obj->GetSize();
    glm::vec3 rotation = obj->GetRotate3D();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat3("Position", &position.x, 0.01f);
        ImGui::DragFloat3("Size", &size.x, 0.5f);
        ImGui::DragFloat3("Rotation", &rotation.x, 0.5f);

        obj->SetPosition(position);
        obj->SetXSize(size.x);
        obj->SetYSize(size.y);
        obj->SetZSize(size.z);
        obj->SetRotate(rotation);
    }

    if (obj->HasComponent<Physics3D>())
    {
        Physics3D* physics = obj->GetComponent<Physics3D>();

        if (ImGui::CollapsingHeader("Physics3D", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::RadioButton("RIGID", physics->GetBodyType() == BodyType3D::RIGID))
            {
                physics->SetBodyType(BodyType3D::RIGID);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("BLOCK", physics->GetBodyType() == BodyType3D::BLOCK))
            {
                physics->SetBodyType(BodyType3D::BLOCK);
            }

            //bool isGhostCollisionOn = physics->GetIsGhostCollision();
            //ImGui::Checkbox("Use GhostCollision", &isGhostCollisionOn);
            //physics->SetIsGhostCollision(isGhostCollisionOn);

            bool isGravityOn = physics->GetIsGravityOn();
            ImGui::Checkbox("Use Gravity", &isGravityOn);
            physics->SetGravity(physics->GetGravity(), isGravityOn);

            glm::vec3 velocity = physics->GetVelocity();
            ImGui::DragFloat3("Velocity", &velocity.x, 0.01f);
            physics->SetVelocity(velocity);

            glm::vec3 minVelocity = physics->GetMinVelocity();
            ImGui::DragFloat3("Min Velocity", &minVelocity.x, 0.01f);
            physics->SetMinVelocity(minVelocity);

            glm::vec3 maxVelocity = physics->GetMaxVelocity();
            ImGui::DragFloat3("Max Velocity", &maxVelocity.x, 0.01f);
            physics->SetMaxVelocity(maxVelocity);

            float gravity = physics->GetGravity();
            ImGui::DragFloat("Gravity", &gravity, 0.01f);
            physics->SetGravity(gravity, isGravityOn);

            float friction = physics->GetFriction();
            ImGui::DragFloat("Friction", &friction, 0.01f);
            physics->SetFriction(friction);
            
            float mass = physics->GetMass();
            ImGui::DragFloat("Mass", &mass, 0.01f);
            physics->SetMass(mass);
        }
    }
    obj = nullptr;
    ImGui::End();

    //SelectObject
    if(Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::LEFT))
    {
        Ray ray = Engine::GetCameraManager().CalculateRayFrom2DPosition(Engine::GetInputManager().GetMousePosition());
        int closestObjectId = lastObjectID;
    	bool isIntersect = false;

    	float tMin, tMax;
        float closestDistance = std::numeric_limits<float>::max();

    	for (auto& object : objectMap) 
        {

            glm::vec3 posTemp = object.second.get()->GetPosition();
            glm::vec3 sizeTemp = object.second.get()->GetSize();

            glm::vec3 invDir = 1.0f / ray.direction;
            glm::vec3 t0 = (posTemp - (sizeTemp / 2.f) - ray.origin) * invDir;
            glm::vec3 t1 = (posTemp + (sizeTemp / 2.f) - ray.origin) * invDir;

            glm::vec3 tMinVec = glm::min(t0, t1);
            glm::vec3 tMaxVec = glm::max(t0, t1);

            tMin = glm::max(glm::max(tMinVec.x, tMinVec.y), tMinVec.z);
            tMax = glm::min(glm::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

            if (tMax >= tMin && tMax >= 0)
            {
                if (tMin < closestDistance) 
                {
                    closestDistance = tMin;
                    closestObjectId = object.second.get()->GetId();
                    isIntersect = true;
                }
            }
        }

        if (isIntersect == true)
        {
            if (currentIndex == closestObjectId)
            {
                ray = Engine::GetCameraManager().CalculateRayFrom2DPosition(Engine::GetInputManager().GetMousePosition());
            	float t = -ray.origin.z / ray.direction.z;
				glm::vec3 intersectionPoint = ray.origin + t * ray.direction;

                Object* objT = FindObjectWithId(currentIndex);
                objT->SetPosition(intersectionPoint);
                objT = nullptr;
            }
            else
            {
                currentIndex = closestObjectId;
            	
                //Object* objT = FindObjectWithId(currentIndex);
                //Engine::GetCameraManager().SetCameraPosition({ objT->GetPosition().x, objT->GetPosition().y , Engine::GetCameraManager().GetCameraPosition().z });
                //objT = nullptr;
            }
        }
    }
    //SelectObject
}
#endif



