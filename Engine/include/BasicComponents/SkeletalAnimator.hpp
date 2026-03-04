//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimator.hpp

#pragma once

#include "Interface/IComponent.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>
#include <memory>
#include <map>
#include <string>

class SkeletalAnimation;
struct AssimpNodeData;

enum class PlaybackState
{
    Playing,
    Paused,
    Stopped
};

struct RootMotionBakeOptions
{
    bool bakePositionX = false;
    bool bakePositionY = true;  // Remove vertical movement by default
    bool bakePositionZ = false;
    bool bakeRotation = true;   // Remove rotation by default
};

class SkeletalAnimator : public IComponent
{
public:
    SkeletalAnimator();

    void Init() override;
    void Update(float dt) override;
    void End() override;

    // Animation Control
    void PlayAnimation(SkeletalAnimation* newAnimation, bool isLoop = true, float speed = 1.f, float blendDuration = 0.25f);
    void Play();
    void Pause();
    void Stop();

    // Getters
    SkeletalAnimation* GetCurrentAnimation() const { return currentAnimation; }
    const std::vector<glm::mat4>& GetFinalBoneMatrices()  const { return finalBoneMatrices; }
    const std::map<std::string, glm::mat4>& GetGlobalBoneTransforms() const { return globalBoneTransforms; }
    float GetCurrentAnimationTime() const { return currentTime; }
    float GetDuration() const;
    float GetSpeed() const { return animationSpeed; }
    const RootMotionBakeOptions& GetBakeOptions() const { return bakeOptions; }
    PlaybackState GetPlaybackState() const { return playbackState; }
    bool IsFinished() const { return justLooped; }
    bool IsLooping() const { return isLooping; }
    bool IsRootMotionEnabled() const { return enableRootMotion; }
    const std::string& GetRootBoneName() const { return rootBoneName; }
    bool IsScrubbing() const { return isScrubbing; }

    // Setters
    void SetLooping(bool loop) { isLooping = loop; };
    void SetCurrentAnimationTime(float time) { currentTime = time; justLooped = false; };
    void SetSpeed(float speed) { animationSpeed = std::max(0.0f, speed); }
    void SetEnableRootMotion(bool enabled) { enableRootMotion = enabled; }
    void SetBakeOptions(const RootMotionBakeOptions& options) { bakeOptions = options; }
    void SetRootBoneName(const std::string& name) { rootBoneName = name; }
    void SetIsScrubbing(bool scrubbing) { isScrubbing = scrubbing; }

    // Apply root motion transform directly for scrubbing or stopping
    void UpdateRootMotionTransformToTime(float time);

private:
    // Calculate target transform relative to time 0
    glm::mat4 CalculateRootMotionTarget(SkeletalAnimation* anim, float time, glm::mat4 startTransform);

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
    bool HasDummyNodeParent(const AssimpNodeData* node, const std::string& boneName) const;
    void CalculateBindPose(const AssimpNodeData* node, glm::mat4 parentTransform);

    // Skinning Data
    std::vector<glm::mat4> finalBoneMatrices;
    std::map<std::string, glm::mat4> globalBoneTransforms;

    // Current Animation State
    SkeletalAnimation* currentAnimation = nullptr;
    PlaybackState playbackState = PlaybackState::Stopped;
    float currentTime = 0.0f;
    float animationSpeed = 1.0f;
    bool  isLooping = true;
    bool  justLooped = false;

    // Blending Data
    SkeletalAnimation* previousAnimation = nullptr;
    float previousTime = 0.0f;
    float blendFactor = 1.0f;
    float currentBlendDuration = 0.0f;

    // Root Motion Data
    bool        enableRootMotion = false;
    bool        isScrubbing = false;
    std::string rootBoneName = "";
    RootMotionBakeOptions bakeOptions;
    glm::mat4 rootMotionStartTransform = glm::mat4(1.0f);
    glm::mat4 previousRootMotionStartTransform = glm::mat4(1.0f);

    // Hierarchy & Bind Pose
    glm::mat4 modelGlobalInverseTransform{ 1.0f };
    std::map<std::string, glm::mat4> bindPoseGlobalTransforms;
    bool bindPoseCalculated = false;

    const int maxBones = 128;
};