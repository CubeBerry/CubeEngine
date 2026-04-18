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
#include <mutex>
#include <iostream>

#include "glm/matrix.hpp"

class DynamicSprite;
class Physics3D;
class Physics2D;
class Light;

class SkeletalAnimator;
class SkeletalAnimationStateMachine;
struct AssimpNodeData;

class ObjectManager
{
public:
    ObjectManager();
    ~ObjectManager();

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

    int   GetLastObjectID();
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
            std::cerr << "nullptr component!" << '\n';
            return;
        }

        std::lock_guard<std::mutex> lock(queueMutex);
        functionQueue.push_back([component, func]()
        {
            func(component);
        });
    }

    template<typename T, typename Func>
    void QueueObjectFunction(T* object, Func&& func) 
    {
        if (object == nullptr) 
        {
            std::cerr << "nullptr object!" << '\n';
            return;
        }
        std::lock_guard<std::mutex> lock(queueMutex);
        functionQueue.push_back([object, func]() 
        {
            func(object);
        });
    }

    void ProcessFunctionQueue();
private:
    void Physics3DControllerForImGui(Physics3D* phy);
    void Physics2DControllerForImGui(Physics2D* phy);
    void RenderPhysics3DDebug(Physics3D* phy);
    void RenderPhysics2DDebug(Physics2D* phy);
    void SpriteControllerForImGui(DynamicSprite* sprite);
    void LightControllerForImGui(Light* light);

    void SkeletalAnimatorControllerForImGui(SkeletalAnimator* animator);
    void AnimationStateMachineControllerForImGui(SkeletalAnimationStateMachine* fsm);
    void RenderBoneHierarchy(const AssimpNodeData* node, const std::map<std::string, glm::mat4>& animatedTransforms, glm::mat4 parentTransform);

    void SelectObjectWithMouse();
    void AddComponentPopUpForImGui();
    void SelectObjModelPopUpForImGui();
    int                                    lastObjectID = 0;
    std::map<int, std::unique_ptr<Object>> objectMap;
    std::vector<int>                       objectsToBeDeleted; // list of object id to be deleted

    //For ObjectController
    std::string objName;
    bool isShowPopup{ false };
    int selectedItem = -1;

    int objectListForImguiIndex = 0;
    int currentIndex = 0;
	int closestObjectId = 0;
    bool isDragObject{ false };
    bool isObjGravityOn{ false };

    int stacks = 2;
    int slices = 2;
    float metallic = 0.3f;
    float roughness = 0.3f;

    std::vector<std::function<void()>> functionQueue;
    std::mutex queueMutex; // Protects functionQueue and objectsToBeDeleted
    //For ObjectController
    
    // Debug Options
    bool isShowBone{ false };
    bool isShowPhysics{ false };
    std::string selectedBoneName = "";
};