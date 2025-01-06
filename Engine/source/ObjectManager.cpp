//Author: DOYEONG LEE
//Project: CubeEngine
//File: ObjectManager.cpp
#include "ObjectManager.hpp"

#include <iostream>

#include "Engine.hpp"
#include "BasicComponents/Physics3D.hpp"
#include "BasicComponents/Light.hpp"
#include "imgui.h"

void ObjectManager::Update(float dt)
{
	std::for_each(objectMap.begin(), objectMap.end(), [&](auto& obj) { obj.second->Update(dt); });
	std::for_each(objectsToBeDeleted.begin(), objectsToBeDeleted.end(), [&](int id) { objectMap.at(id).reset(); objectMap.erase(id); });
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
    //objectsToBeDeleted.push_back(id);
    //objectMap.at(id).get()->DestroyAllComponents();

    objectsToBeDeleted.push_back(id);
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

    for (auto* comp : obj->GetComponentList())
    {
        ComponentTypes type = comp->GetType();
        switch (type)
        {
        case ComponentTypes::PHYSICS3D:
            Physics3DControllerForImGui(reinterpret_cast<Physics3D*>(comp));
            break;
        case ComponentTypes::LIGHT:
            LightControllerForImGui(reinterpret_cast<Light*>(comp));
            break;
        }
    }

    obj = nullptr;
    ImGui::End();

    SelectObjectWithMouse();
}

void ObjectManager::Physics3DControllerForImGui(Physics3D* phy)
{
    Physics3D* physics = phy;
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
    physics = nullptr;
}

void ObjectManager::LightControllerForImGui(Light* light)
{
    glm::vec3 position = light->GetPosition();
    glm::vec4 rotation = light->GetRotate();
    glm::vec4 color = light->GetColor();

    float ambient = light->GetAmbientStrength();
    float specular = light->GetSpecularStrength();

    if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if(light->GetLightType() == LightType::Point)
        {
            ImGui::DragFloat3("Light Position", &position.x, 0.01f);
            ImGui::DragFloat3("Light Rotation", &rotation.x, 0.5f);

            light->SetXPosition(position.x);
            light->SetYPosition(position.y);
            light->SetZPosition(position.z);
            if(light->GetRotate() != rotation)
            {
                light->SetRotate(rotation);
            }
        }
        else if (light->GetLightType() == LightType::Direct)
        {
            ImGui::DragFloat3("Light Direction", &rotation.x, 0.5f);
            if (light->GetRotate() != rotation)
            {
                light->SetRotate(rotation);
            }
        }

        ImGui::ColorPicker3("Light Color", &color.r);
        ImGui::SliderFloat("Ambient Strength", &ambient, 0.f, 1.f);
        ImGui::SliderFloat("Specular Strength", &specular, 0.f, 1.f);

        light->SetColor(color);
        light->SetAmbientStrength(ambient);
        light->SetSpecularStrength(specular);
    }
}

void ObjectManager::SelectObjectWithMouse()
{
    //SelectObject
    if (Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::LEFT))
    {
        Ray ray = Engine::GetCameraManager().CalculateRayFrom2DPosition(Engine::GetInputManager().GetMousePosition());
        if (isDragObject == false)
        {
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
                        isDragObject = true;
                    }
                }
            }
        }
        else
        {
            if (currentIndex == closestObjectId)
            {
                Object* objT = FindObjectWithId(closestObjectId);

                if (isObjGravityOn == false && objT->HasComponent<Physics3D>() && objT->GetComponent<Physics3D>()->GetIsGravityOn() == true)
                {
                    objT->GetComponent<Physics3D>()->SetIsGravityOn(false);
                    isObjGravityOn = true;
                }
                ray = Engine::GetCameraManager().CalculateRayFrom2DPosition(Engine::GetInputManager().GetMousePosition());
                glm::vec3 planeNormal = Engine::GetCameraManager().GetBackVector();
                glm::vec3 objectPosition = objT->GetPosition();
                float distanceToPlane = glm::dot(planeNormal, objectPosition - ray.origin) / glm::dot(planeNormal, ray.direction);
                glm::vec3 intersectionPoint = ray.origin + distanceToPlane * ray.direction;

                objT->SetPosition(intersectionPoint);
                objT = nullptr;
            }
            else
            {
                currentIndex = closestObjectId;
            }
        }
    }
    else if (isDragObject == true && !Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::LEFT))
    {
        if (isObjGravityOn == true)
        {
            Object* objT = FindObjectWithId(closestObjectId);
            if (objT->HasComponent<Physics3D>())
            {
                objT->GetComponent<Physics3D>()->SetIsGravityOn(true);
                isObjGravityOn = false;
            }
            objT = nullptr;
        }
        closestObjectId = 0;
        isDragObject = false;
    }
    //SelectObject
}



