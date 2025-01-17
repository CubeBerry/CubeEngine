//Author: DOYEONG LEE
//Second Author: JEYOON YU
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
    currentIndex = 0;
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
    //if (ImGui::Button("Add New Object"))
    //{
    //    AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, "NEW OBJECT", ObjectType::NONE);
    //    GetLastObject()->AddComponent<Sprite>();
    //    GetLastObject()->GetComponent<Sprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1);
    //}
    ImGui::Separator();
    ImGui::Spacing();

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
        ImGui::Separator();
        ImGui::Spacing();

    if(!objectMap.empty())
    {
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

        ImGui::Separator();
        ImGui::Spacing();
        /*if (ImGui::Button("Add Component"))
        {
            isShowPopup = true;
            ImGui::OpenPopup("Select Component");
        }*/

        obj = nullptr;

        if (ImGui::IsPopupOpen("Select Component"))
        {
            AddComponentPopUpForImGui();
        }
    }

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
    if(light->GetLightType() != LightType::NONE)
    {
        glm::vec3 position = light->GetPosition();
        glm::vec4 rotation = light->GetRotate();
        glm::vec4 color = light->GetColor();

        float ambient = light->GetAmbientStrength();
        float specular = light->GetSpecularStrength();


        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::ColorEdit4("Light Color", &color.r))
            {
                light->SetColor(color);
            }
            ImGui::Spacing();

            if (light->GetLightType() == LightType::DIRECTIONAL)
            {
                ImGui::DragFloat3("Light Direction", &position.x, 0.01f);
                ImGui::Spacing();

                light->SetXPosition(position.x);
                light->SetYPosition(position.y);
                light->SetZPosition(position.z);
            }
            if (light->GetLightType() == LightType::POINT)
            {
                float constant = light->GetConstant();
                float linear = light->GetLinear();
                float quadratic = light->GetQuadratic();

                ImGui::DragFloat3("Light Position", &position.x, 0.01f);
                ImGui::Spacing();
                //ImGui::DragFloat3("Light Rotation", &rotation.x, 0.5f);

                ImGui::SliderFloat("constant", &constant, 0.f, 1.f);
                ImGui::SliderFloat("linear", &linear, 0.f, 1.f);
                ImGui::SliderFloat("quadratic", &quadratic, 0.f, 1.f);

                light->SetXPosition(position.x);
                light->SetYPosition(position.y);
                light->SetZPosition(position.z);
                /*if(light->GetRotate() != rotation)
                {
                    light->SetRotate(rotation);
                }*/

                light->SetConstant(constant);
                light->SetLinear(linear);
                light->SetQuadratic(quadratic);
            }

            ImGui::SliderFloat("Ambient Strength", &ambient, 0.f, 1.f);
            ImGui::SliderFloat("Specular Strength", &specular, 0.f, 1.f);

            light->SetAmbientStrength(ambient);
            light->SetSpecularStrength(specular);
        }
    }
    else 
    {
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        ImVec2 popupSize = ImVec2(200, 300);
        ImVec2 popupPos = ImVec2((displaySize.x - popupSize.x) / 2.0f, (displaySize.y - popupSize.y) / 2.0f);

        ImGui::SetNextWindowPos(popupPos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(popupSize);

        if (!ImGui::IsPopupOpen("Select Light Type"))
        {
            isShowPopup = true;
            ImGui::OpenPopup("Select Light Type");
        }

        if (ImGui::BeginPopupModal("Select Light Type", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Direct"))
            {
                isShowPopup = false;
                light->AddLight(LightType::DIRECTIONAL);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Point"))
            {
                isShowPopup = false;
                light->AddLight(LightType::POINT);
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();
            ImGui::EndPopup();
        }

        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Background Overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
        ImGui::End();
    }
}

void ObjectManager::SelectObjectWithMouse()
{
    //SelectObject
    if (Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::LEFT) && isShowPopup == false)
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

                //objT->SetPosition(intersectionPoint);
                objT = nullptr;
            }
            else
            {
                currentIndex = closestObjectId;
            }
        }
    }
    else if (isDragObject == true && !Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::LEFT) && isShowPopup == false)
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

void ObjectManager::AddComponentPopUpForImGui()
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImVec2 popupSize = ImVec2(400, 300);
    ImVec2 popupPos = ImVec2((displaySize.x - popupSize.x) / 2.0f, (displaySize.y - popupSize.y) / 2.0f );

    ImGui::SetNextWindowPos(popupPos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(popupSize);

    if (ImGui::BeginPopupModal("Select Component", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, 3 * ImGui::GetTextLineHeightWithSpacing()));
        ImGui::BeginChild("Scolling");
        Object* currentObj = FindObjectWithId(currentIndex);

        for (int i = 0; i < static_cast<int>(ComponentTypes::INVALID); i++)
        {
            if (ImGui::Selectable(ComponentToString(static_cast<ComponentTypes>(i)).c_str(), selectedItem == i))
            {
                selectedItem = i;

                switch (static_cast<ComponentTypes>(i))
                {
                case ComponentTypes::SPRITE:
                    if(currentObj->HasComponent<Sprite>() == false)
                    {
                        currentObj->AddComponent<Sprite>();
                    }
                    break;
                case ComponentTypes::PHYSICS2D:
                    if (currentObj->HasComponent<Physics2D>() == false)
                    {
                        currentObj->AddComponent<Physics2D>();
                    }
                    break;
                case ComponentTypes::PHYSICS3D:
                    if (currentObj->HasComponent<Physics3D>() == false)
                    {
                        currentObj->AddComponent<Physics3D>();
                    }
                    break;
                case ComponentTypes::LIGHT:
                    if (currentObj->HasComponent<Light>() == false)
                    {
                        currentObj->AddComponent<Light>();
                    }
                    break;
                }
                isShowPopup = false;
                ImGui::CloseCurrentPopup();
            }
        }
        currentObj = nullptr;
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Close"))
        {
            isShowPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Background Overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
    ImGui::End();
}



