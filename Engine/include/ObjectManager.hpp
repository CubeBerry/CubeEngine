//Author: DOYEONG LEE
//Project: CubeEngine
//File: ObjectManager.hpp
#pragma once
#include "Object.hpp"

#include <functional>
#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include <iostream>

class Physics3D;
class Light;
class ObjectManager
{
public:
    ObjectManager()  = default;
    ~ObjectManager() = default;

	void Update(float dt);
    void DeleteObjectsFromList();
    void End();

    template <typename T, typename... Args>
    void AddObject(Args... args)
    {
        //auto object = std::unique_ptr<T>(std::make_unique<T>(std::forward<Args>(args)...));
        ////T& objectRef = *object.get();
        //static_cast<Object&>(*object.get()).SetId(lastObjectID);

        objectMap[lastObjectID] = std::unique_ptr<T>(std::make_unique<T>(std::forward<Args>(args)...));;
        objectMap[lastObjectID].get()->SetId(lastObjectID);
        ++lastObjectID;
        //return static_cast<T&>(*objectMap[lastObjectID - 1].get());
    }

    void  Draw(float dt);
    void  Destroy(int id);
    void  DestroyAllObjects();

    int   GetLastObjectID() { return (GetLastObject()->GetId()); }
    std::map<int, std::unique_ptr<Object>>& GetObjectMap() { return objectMap; }

    Object* FindObjectWithName(std::string name);
    Object* FindObjectWithId(int id) { return objectMap.at(id).get(); }
    Object* GetLastObject()
    {
        if (objectMap.empty())
        {
            return nullptr;
        }
        return objectMap.rbegin()->second.get();
    }
    void ObjectControllerForImGui();

    template<typename ComponentTypes, typename Func>
    void QueueComponentFunction(ComponentTypes* component, Func&& func)
    {
        if (component == nullptr)
        {
            std::cerr << "nullptr component!" << std::endl;
            return;
        }

        componentFunctionQueue.push_back([component, func]()
        {
            func(component);
        });
    }

    void ProcessComponentFunctionQueues();
private:
    void Physics3DControllerForImGui(Physics3D* phy);
    void LightControllerForImGui(Light* light);
    void SelectObjectWithMouse();
    void AddComponentPopUpForImGui();
    int                                    lastObjectID = 0;
    std::map<int, std::unique_ptr<Object>> objectMap;
    std::vector<int>                       objectsToBeDeleted; // list of object id to be deleted

    //For ObjectController
    bool isShowPopup = false;
    int selectedItem = -1;

    int objectListForImguiIndex = 0;
    int currentIndex = 0;
	int closestObjectId = 0;
    bool isDragObject = false;
    bool isObjGravityOn = false;
    //

    std::vector<std::function<void()>> componentFunctionQueue;
};