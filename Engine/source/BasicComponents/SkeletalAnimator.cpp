//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimator.cpp

#define NOMINMAX

#include "BasicComponents/SkeletalAnimator.hpp"
#include "BasicComponents/DynamicSprite.hpp"
#include "BufferWrapper.hpp"
#include "Object.hpp"
#include "SkeletalAnimation/SkeletalAnimation.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <algorithm>
#include <functional>

SkeletalAnimator::SkeletalAnimator()
    : IComponent(ComponentTypes::SKETANIMATOR),
    currentTime(0.0f),
    currentAnimation(nullptr),
    previousAnimation(nullptr)
{
    finalBoneMatrices.reserve(ThreeDimension::MAX_BONES);
    for (int i = 0; i < ThreeDimension::MAX_BONES; i++)
        finalBoneMatrices.push_back(glm::mat4(1.0f));
}

void SkeletalAnimator::Init() {}
void SkeletalAnimator::End() {}

void SkeletalAnimator::Update(float dt)
{
    if (!currentAnimation) return;

    // Update blending factor
    if (currentBlendDuration <= 0.0f) currentBlendDuration = 0.01f;
    blendFactor += dt / currentBlendDuration;
    if (blendFactor >= 1.0f)
    {
        blendFactor = 1.0f;
        previousAnimation = nullptr;
    }

    // Update animation playback time
    if (playbackState == PlaybackState::Playing)
    {
        // Update previous animation time during blending
        if (previousAnimation && blendFactor < 1.0f)
        {
            previousTime += dt * animationSpeed * previousAnimation->GetTicksPerSecond();
            float prevDuration = previousAnimation->GetDuration();
            if (isLooping) previousTime = fmod(previousTime, prevDuration);
            else if (previousTime >= prevDuration) previousTime = prevDuration;
        }

        if (!isScrubbing)
        {
            currentTime += dt * animationSpeed * currentAnimation->GetTicksPerSecond();

            float duration = currentAnimation->GetDuration();
            if (currentTime >= duration)
            {
                justLooped = true;
                if (isLooping)
                    currentTime = fmod(currentTime, duration);
                else
                {
                    currentTime = duration - 0.001f;
                    playbackState = PlaybackState::Stopped;
                }
            }
        }
    }

    // Apply Root Motion
    if (enableRootMotion && playbackState == PlaybackState::Playing && !isScrubbing)
    {
        // Calculate target transform relative to start
        glm::mat4 currentTarget = CalculateRootMotionTarget(currentAnimation, currentTime, rootMotionStartTransform);
        glm::mat4 finalTarget = currentTarget;

        // Blend root motion transforms
        if (blendFactor < 1.0f && previousAnimation)
        {
            glm::mat4 previousTarget = CalculateRootMotionTarget(previousAnimation, previousTime, previousRootMotionStartTransform);

            glm::vec3 prevPos, currPos, prevScale, currScale, prevSkew, currSkew;
            glm::quat prevRot, currRot;
            glm::vec4 prevPers, currPers;
            glm::decompose(previousTarget, prevScale, prevRot, prevPos, prevSkew, prevPers);
            glm::decompose(currentTarget, currScale, currRot, currPos, currSkew, currPers);

            glm::vec3 finalPos = glm::mix(prevPos, currPos, blendFactor);
            glm::quat finalRot = glm::slerp(prevRot, currRot, blendFactor);
            glm::vec3 finalScale = glm::mix(prevScale, currScale, blendFactor);

            finalTarget = glm::translate(glm::mat4(1.0f), finalPos)
                * glm::mat4_cast(finalRot)
                * glm::scale(glm::mat4(1.0f), finalScale);
        }

        // Apply result to owner object
        Object* owner = GetOwner();
        if (owner)
        {
            glm::vec3 pos, scale, skew;
            glm::quat rot;
            glm::vec4 pers;
            glm::decompose(finalTarget, scale, rot, pos, skew, pers);
            owner->SetPosition(pos);
            owner->SetRotate(glm::degrees(glm::eulerAngles(rot)));
        }
    }

    // Ensure bone array capacity
    Object* owner = GetOwner();
    if (owner)
    {
        DynamicSprite* sprite = owner->GetComponent<DynamicSprite>();
        if (sprite && !sprite->GetSubMeshes().empty())
        {
            auto* meshData = sprite->GetSubMeshes()[0]->GetData<BufferWrapper::DynamicSprite3DMesh>();
            if (meshData && finalBoneMatrices.size() < meshData->boneInfoMap.size())
                finalBoneMatrices.resize(meshData->boneInfoMap.size(), glm::mat4(1.0f));
        }
    }

    // Calculate bone hierarchy transforms
    CalculateBoneTransform(&currentAnimation->GetRootNode(), glm::mat4(1.0f));

    // Upload bone matrices to GPU
    if (owner)
    {
        DynamicSprite* sprite = owner->GetComponent<DynamicSprite>();
        if (sprite)
        {
            for (auto& subMesh : sprite->GetSubMeshes())
            {
                auto* meshData = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>();
                if (!meshData) continue;

                int count = (std::min)((int)finalBoneMatrices.size(), ThreeDimension::MAX_BONES);
                for (int i = 0; i < count; ++i)
                    meshData->vertexUniform.finalBones[i] = finalBoneMatrices[i];

                if (std::holds_alternative<std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>>(meshData->vertexUniformBuffer))
                {
                    auto& glBuffer = std::get<std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>>(meshData->vertexUniformBuffer);
                    glBuffer->UpdateUniform(sizeof(ThreeDimension::VertexUniform), &meshData->vertexUniform);
                }
            }
        }
    }
}

void SkeletalAnimator::PlayAnimation(SkeletalAnimation* newAnimation, bool isLoop, float speed, float blendDuration)
{
    if (currentAnimation == newAnimation) return;

    Object* owner = GetOwner();

    // Setup blending state
    if (blendDuration > 0.0f && currentAnimation != nullptr)
    {
        previousAnimation = currentAnimation;
        previousTime = currentTime;
        if (enableRootMotion)
            previousRootMotionStartTransform = rootMotionStartTransform;
        blendFactor = 0.0f;
        currentBlendDuration = blendDuration;
    }
    else
    {
        previousAnimation = nullptr;
        blendFactor = 1.0f;
    }

    // Initialize new animation state
    currentAnimation = newAnimation;
    currentTime = 0.0f;
    isLooping = isLoop;
    animationSpeed = speed;
    justLooped = false;
    playbackState = PlaybackState::Playing;

    // Update root motion start transform to current owner position
    if (enableRootMotion && owner)
    {
        glm::vec3 pos = owner->GetPosition();
        glm::vec3 rot = owner->GetRotate3D();
        glm::vec3 scl = owner->GetSize();

        rootMotionStartTransform = glm::translate(glm::mat4(1.0f), pos)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rot.x), glm::vec3(1, 0, 0))
            * glm::rotate(glm::mat4(1.0f), glm::radians(rot.y), glm::vec3(0, 1, 0))
            * glm::rotate(glm::mat4(1.0f), glm::radians(rot.z), glm::vec3(0, 0, 1))
            * glm::scale(glm::mat4(1.0f), scl);
    }

    // Auto-detect root bone name
    if (enableRootMotion && rootBoneName.empty() && newAnimation)
    {
        const std::vector<std::string> commonRootNames = { "Hips", "hips", "Root", "root" };
        for (const auto& bone : newAnimation->GetBones())
        {
            for (const auto& commonName : commonRootNames)
            {
                if (bone.GetBoneName().find(commonName) != std::string::npos)
                {
                    std::string baseName = bone.GetBoneName();
                    size_t pos = baseName.find("_$");
                    if (pos != std::string::npos) baseName = baseName.substr(0, pos);
                    rootBoneName = baseName;
                    goto rootBoneFound;
                }
            }
        }
    rootBoneFound:;
    }

    // Calculate bind pose on first run
    if (!bindPoseCalculated && currentAnimation)
    {
        CalculateBindPose(&currentAnimation->GetRootNode(), glm::mat4(1.0f));
        bindPoseCalculated = true;
    }
}

void SkeletalAnimator::Play()
{
    playbackState = PlaybackState::Playing;
}

void SkeletalAnimator::Pause()
{
    playbackState = PlaybackState::Paused;
}

void SkeletalAnimator::Stop()
{
    playbackState = PlaybackState::Stopped;
    currentTime = 0.0f;
    if (enableRootMotion)
        UpdateRootMotionTransformToTime(0.0f);
}

glm::mat4 SkeletalAnimator::CalculateRootMotionTarget(SkeletalAnimation* anim, float time, glm::mat4 startTransform)
{
    if (!anim || rootBoneName.empty()) return startTransform;

    // Define potential bone names for FBX/Mocap formats
    std::string posBoneName = rootBoneName + "_$AssimpFbx$_Translation";
    std::string rotBoneName = rootBoneName + "_$AssimpFbx$_Rotation";
    std::string rotBoneNameMocap = rootBoneName + "_$AssMocapFix$_Rotation";

    SkeletalBone* posBone = anim->FindBone(posBoneName);
    SkeletalBone* rotBone = anim->FindBone(rotBoneName);
    if (!rotBone) rotBone = anim->FindBone(rotBoneNameMocap);

    // Fallback to root bone if translation dummy is missing
    if (!posBone) posBone = anim->FindBone(rootBoneName);

    if (!posBone && !rotBone) return startTransform;

    // Local Transform at time = 0 (Reference Point)
    glm::vec3 startPos = posBone ? posBone->GetInterpolatedPosition(0.0f) : glm::vec3(0.f);
    glm::quat startRot = rotBone ? rotBone->GetInterpolatedRotation(0.0f) : glm::quat(1, 0, 0, 0);
    glm::mat4 startBoneT = glm::translate(glm::mat4(1.0f), startPos) * glm::mat4_cast(startRot);

    // Local Transform at current time
    glm::vec3 targetPos = posBone ? posBone->GetInterpolatedPosition(time) : glm::vec3(0.f);
    glm::quat targetRot = rotBone ? rotBone->GetInterpolatedRotation(time) : glm::quat(1, 0, 0, 0);
    glm::mat4 targetBoneT = glm::translate(glm::mat4(1.0f), targetPos) * glm::mat4_cast(targetRot);

    // Change relative to the start of the animation
    glm::mat4 delta = glm::inverse(startBoneT) * targetBoneT;

    // Apply bake options (lock specific axes)
    if (bakeOptions.bakePositionX) delta[3][0] = 0.0f;
    if (bakeOptions.bakePositionY) delta[3][1] = 0.0f;
    if (bakeOptions.bakePositionZ) delta[3][2] = 0.0f;
    if (bakeOptions.bakeRotation)
    {
        delta[0] = glm::vec4(1, 0, 0, 0);
        delta[1] = glm::vec4(0, 1, 0, 0);
        delta[2] = glm::vec4(0, 0, 1, 0);
    }

    // Start World Transform * Relative Animation Delta
    return startTransform * delta;
}

void SkeletalAnimator::UpdateRootMotionTransformToTime(float time)
{
    if (!enableRootMotion || !currentAnimation) return;

    Object* owner = GetOwner();
    if (!owner) return;

    glm::mat4 finalTransform = CalculateRootMotionTarget(currentAnimation, time, rootMotionStartTransform);

    glm::vec3 pos, scale, skew;
    glm::quat rot;
    glm::vec4 pers;
    glm::decompose(finalTransform, scale, rot, pos, skew, pers);

    owner->SetPosition(pos);
    owner->SetRotate(glm::degrees(glm::eulerAngles(rot)));
}

void SkeletalAnimator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    bool isDummyNode = (nodeName.find("_$AssimpFbx$_") != std::string::npos);

    if (isDummyNode)
    {
        // Translation Dummy: Apply position from animation
        if (nodeName.find("_$AssimpFbx$_Translation") != std::string::npos)
        {
            std::string boneName = nodeName.substr(0, nodeName.find("_$AssimpFbx$_"));

            SkeletalBone* bone = currentAnimation ? currentAnimation->FindBone(boneName) : nullptr;
            SkeletalBone* prevBone = (blendFactor < 1.0f && previousAnimation)
                ? previousAnimation->FindBone(boneName) : nullptr;

            if (bone)
            {
                bone->Update(currentTime);
                glm::vec3 p1 = bone->GetInterpolatedPosition(currentTime);

                if (!prevBone)
                {
                    nodeTransform = node->transformation;
                    nodeTransform[3] = glm::vec4(p1, 1.0f);
                }
                else
                {
                    prevBone->Update(previousTime);
                    glm::vec3 p2 = prevBone->GetInterpolatedPosition(previousTime);
                    glm::vec3 p = glm::mix(p2, p1, blendFactor);
                    nodeTransform = node->transformation;
                    nodeTransform[3] = glm::vec4(p, 1.0f);
                }
            }
        }

        // Root Motion: Reset dummy transforms so the mesh stays centered
        // (The Object Transform handles the world movement)
        if (enableRootMotion)
        {
            std::string translationChannel = rootBoneName + "_$AssimpFbx$_Translation";
            std::string rotationChannel = rootBoneName + "_$AssimpFbx$_Rotation";
            std::string rotationChannelMocap = rootBoneName + "_$AssMocapFix$_Rotation";

            if (nodeName == translationChannel)
            {
                nodeTransform[3][0] = 0.0f;
                nodeTransform[3][1] = 0.0f;
                nodeTransform[3][2] = 0.0f;
            }
            if (nodeName == rotationChannel || nodeName == rotationChannelMocap)
            {
                nodeTransform = glm::mat4(1.0f);
            }
        }
    }
    else
    {
        // Standard Bone Node
        SkeletalBone* bone = currentAnimation ? currentAnimation->FindBone(nodeName) : nullptr;
        SkeletalBone* prevBone = (blendFactor < 1.0f && previousAnimation)
            ? previousAnimation->FindBone(nodeName) : nullptr;

        if (bone)
        {
            bone->Update(currentTime);

            // Extract and remove PreRotation to get pure Local Rotation
            glm::quat preRot = glm::quat(1.f, 0.f, 0.f, 0.f);
            const auto& preRotMap = currentAnimation->GetPreRotations();
            auto preIt = preRotMap.find(nodeName);
            if (preIt != preRotMap.end())
                preRot = preIt->second;

            // Position follows bind pose (Translation dummies handle movement)
            glm::vec3 p1 = glm::vec3(node->transformation[3]);

            // Rotation blending and calculation
            glm::quat animRot = bone->GetInterpolatedRotation(currentTime);
            glm::quat r1 = glm::normalize(glm::inverse(preRot) * animRot);
            glm::vec3 s1 = bone->GetInterpolatedScale(currentTime);

            if (!prevBone)
            {
                nodeTransform = glm::translate(glm::mat4(1.0f), p1)
                    * glm::mat4_cast(r1)
                    * glm::scale(glm::mat4(1.0f), s1);
            }
            else
            {
                prevBone->Update(previousTime);
                glm::vec3 p2 = glm::vec3(node->transformation[3]);
                glm::quat prevAnimRot = prevBone->GetInterpolatedRotation(previousTime);
                glm::quat r2 = glm::normalize(glm::inverse(preRot) * prevAnimRot);
                glm::vec3 s2 = prevBone->GetInterpolatedScale(previousTime);

                glm::vec3 p = glm::mix(p2, p1, blendFactor);
                if (glm::dot(r2, r1) < 0.0f) r1 = -r1; // Shortest path rotation
                glm::quat r = glm::normalize(glm::slerp(r2, r1, blendFactor));
                glm::vec3 s = glm::mix(s2, s1, blendFactor);

                nodeTransform = glm::translate(glm::mat4(1.0f), p)
                    * glm::mat4_cast(r)
                    * glm::scale(glm::mat4(1.0f), s);
            }
        }
    }

    // Update global hierarchy and store transform
    glm::mat4 globalTransformation = parentTransform * nodeTransform;
    globalBoneTransforms[nodeName] = globalTransformation;

    // Send final bone matrices to the GPU buffer
    if (currentAnimation)
    {
        Object* owner = GetOwner();
        if (owner)
        {
            DynamicSprite* sprite = owner->GetComponent<DynamicSprite>();
            if (sprite && !sprite->GetSubMeshes().empty())
            {
                auto* meshData = sprite->GetSubMeshes()[0]
                    ->GetData<BufferWrapper::DynamicSprite3DMesh>();
                if (meshData && meshData->boneInfoMap.find(nodeName) != meshData->boneInfoMap.end())
                {
                    int index = meshData->boneInfoMap.at(nodeName).id;
                    glm::mat4 offset = meshData->boneInfoMap.at(nodeName).offset;
                    glm::mat4 encodeMatrix = meshData->meshNormalizationTransform;
                    if (index < (int)finalBoneMatrices.size())
                        finalBoneMatrices[index] = encodeMatrix * globalTransformation * offset;
                }
            }
        }
    }

    // Recursively process children
    for (const auto& child : node->children)
        CalculateBoneTransform(&child, globalTransformation);
}

bool SkeletalAnimator::HasDummyNodeParent(const AssimpNodeData* node, const std::string& boneName) const
{
    if (!currentAnimation) return false;

    std::function<const AssimpNodeData* (const AssimpNodeData*, const std::string&)> findParent;
    findParent = [&](const AssimpNodeData* current, const std::string& targetName) -> const AssimpNodeData*
    {
        for (const auto& child : current->children)
        {
            if (child.name == targetName) return current;
            auto* result = findParent(&child, targetName);
            if (result) return result;
        }
        return nullptr;
    };

    const AssimpNodeData* parent = findParent(&currentAnimation->GetRootNode(), boneName);
    if (!parent) return false;
    return parent->name.find("_$AssimpFbx$_") != std::string::npos;
}

void SkeletalAnimator::CalculateBindPose(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    glm::mat4 globalTransform = parentTransform * node->transformation;
    bindPoseGlobalTransforms[node->name] = globalTransform;
    for (const auto& child : node->children)
        CalculateBindPose(&child, globalTransform);
}

float SkeletalAnimator::GetDuration() const 
{ 
    return currentAnimation ? currentAnimation->GetDuration() : 0.0f; 
}