//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimationStateMachine.hpp

#pragma once

#include "Interface/IComponent.hpp"
#include <string>
#include <map>
#include <memory>
#include <vector>

class SkeletalAnimation;
class SkeletalAnimator;

class SkeletalAnimationStateMachine : public IComponent
{
public:
    SkeletalAnimationStateMachine();

    void Init() override;
    void Update(float dt) override;
    void End() override;

    // Use an existing animation shared from external sources like a Scene
    void AddState(const std::string& name, std::shared_ptr<SkeletalAnimation> anim);

    // Create and own an animation directly from a file path
    void AddState(const std::string& name, const std::string& animationFilePath);

    void ChangeState(const std::string& name, bool isLoop = true, float speed = 1.f, float blendDuration = 0.25f);

    std::vector<std::string> GetStateNames() const;
    std::string GetCurrentStateName() const;

private:
    // Internal class representing a single animation state
    class State
    {
    public:
        State(std::shared_ptr<SkeletalAnimation> anim) : animation(anim) {}
        std::shared_ptr<SkeletalAnimation> animation;
    };

    std::map<std::string, std::unique_ptr<State>> states;
    State* currentState = nullptr;
    SkeletalAnimator* animator = nullptr;
};