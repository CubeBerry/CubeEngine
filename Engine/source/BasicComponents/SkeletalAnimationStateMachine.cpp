//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimationStateMachine.cpp

// Define NOMINMAX to prevent macro collisions
#define NOMINMAX

#include "BasicComponents/SkeletalAnimationStateMachine.hpp"
#include "Object.hpp"
#include "BasicComponents/SkeletalAnimator.hpp"
#include "SkeletalAnimation/SkeletalAnimation.hpp"
#include "BasicComponents/DynamicSprite.hpp"
#include "BufferWrapper.hpp"

#include <iostream> // For debugging

SkeletalAnimationStateMachine::SkeletalAnimationStateMachine() : IComponent(ComponentTypes::SKETANIMASTATE) {}

void SkeletalAnimationStateMachine::Init()
{
    // Find and store the Animator component from the owner object
    // The FSM must work in pair with a SkeletalAnimator
    animator = GetOwner()->GetComponent<SkeletalAnimator>();
}

void SkeletalAnimationStateMachine::Update(float /*dt*/)
{
    // Update is empty because the Animator handles timing and matrix updates independently
}

void SkeletalAnimationStateMachine::End() {}

void SkeletalAnimationStateMachine::AddState(const std::string& name, std::shared_ptr<SkeletalAnimation> anim)
{
    // Create a new State with the provided animation pointer and add it to the map
    states.emplace(name, std::make_unique<State>(anim));
    currentState = states[name].get();
}

void SkeletalAnimationStateMachine::AddState(const std::string& name, const std::string& animationFilePath)
{
    Object* owner_ = GetOwner();
    if (!owner_) return;

    DynamicSprite* sprite = owner_->GetComponent<DynamicSprite>();
    if (!sprite) return;

    // Access the specific submesh data that contains the bone information
    auto& subMeshes = sprite->GetSubMeshes();
    if (subMeshes.empty()) return;

    // Assume the character mesh is the first submesh
    auto* meshData = subMeshes[0]->GetData<BufferWrapper::DynamicSprite3DMesh>();
    if (!meshData) return;

    // Create a new animation using the mesh data and add it to the states map
    auto animation = std::make_shared<SkeletalAnimation>(animationFilePath, meshData);
    states.emplace(name, std::make_unique<State>(animation));
}

void SkeletalAnimationStateMachine::ChangeState(const std::string& name, bool isLoop, float speed, float blendDuration)
{
    // Ensure the animator pointer is valid before changing states
    if (animator == nullptr)
    {
        animator = GetOwner()->GetComponent<SkeletalAnimator>();
    }

    if (states.count(name))
    {
        State* nextState = states[name].get();

        // Only change if the requested state is different from the current one
        if (currentState != nextState)
        {
            currentState = nextState;

            if (animator)
            {
                animator->PlayAnimation(currentState->animation.get(), isLoop, speed, blendDuration);
                std::cout << "Animation state changed to: " << name << std::endl;
            }
        }
        else
        {
            // If the state is the same but no animation is playing, trigger it
            if (animator && animator->GetCurrentAnimation() == nullptr)
            {
                animator->PlayAnimation(currentState->animation.get(), isLoop, speed, blendDuration);
            }
        }
    }
}

std::vector<std::string> SkeletalAnimationStateMachine::GetStateNames() const
{
    // Collect and return all keys from the states map
    std::vector<std::string> names;
    for (const auto& pair : states)
    {
        names.push_back(pair.first);
    }
    return names;
}

std::string SkeletalAnimationStateMachine::GetCurrentStateName() const
{
    if (currentState == nullptr)
    {
        return "None";
    }

    // Search the map to find the name corresponding to the current state pointer
    for (const auto& pair : states)
    {
        if (pair.second.get() == currentState)
        {
            return pair.first;
        }
    }
    return "Unknown";
}