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

    bool currentJustLooped = false;
    bool previousJustLooped = false;
    float oldPreviousTime = previousTime;
    float oldCurrentTime = currentTime;

    // Update animation playback time
    if (playbackState == PlaybackState::Playing)
    {
        // Update previous animation time during blending
        if (previousAnimation && blendFactor < 1.0f)
        {
            float deltaAnimTime = dt * animationSpeed * previousAnimation->GetTicksPerSecond();
            previousTime += deltaAnimTime;
            float prevDuration = previousAnimation->GetDuration();
            if (previousTime >= prevDuration)
            {
                previousJustLooped = true;
                if (isLooping) previousTime = fmod(previousTime, prevDuration);
                else previousTime = prevDuration - 0.001f;
            }
        }

        if (!isScrubbing)
        {
            float deltaAnimTime = dt * animationSpeed * currentAnimation->GetTicksPerSecond();
            currentTime += deltaAnimTime;

            float duration = currentAnimation->GetDuration();
            if (currentTime >= duration)
            {
                currentJustLooped = true;
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
        glm::mat4 currentDelta = glm::mat4(1.0f);
        if (currentJustLooped && isLooping)
        {
            glm::mat4 d1 = ExtractRootMotionDelta(currentAnimation, oldCurrentTime, currentAnimation->GetDuration());
            glm::mat4 d2 = ExtractRootMotionDelta(currentAnimation, 0.0f, currentTime);
            currentDelta = d1 * d2;
        }
        else
        {
            currentDelta = ExtractRootMotionDelta(currentAnimation, oldCurrentTime, currentTime);
        }

        glm::mat4 finalDelta = currentDelta;

        if (blendFactor < 1.0f && previousAnimation)
        {
            glm::mat4 previousDelta = glm::mat4(1.0f);
            if (previousJustLooped && isLooping)
            {
                glm::mat4 d1 = ExtractRootMotionDelta(previousAnimation, oldPreviousTime, previousAnimation->GetDuration());
                glm::mat4 d2 = ExtractRootMotionDelta(previousAnimation, 0.0f, previousTime);
                previousDelta = d1 * d2;
            }
            else
            {
                previousDelta = ExtractRootMotionDelta(previousAnimation, oldPreviousTime, previousTime);
            }

            glm::vec3 prevDT, currDT;
            glm::quat prevDR, currDR;
            glm::vec3 dS, dSk; glm::vec4 dP;
            glm::decompose(previousDelta, dS, prevDR, prevDT, dSk, dP);
            glm::decompose(currentDelta, dS, currDR, currDT, dSk, dP);

            glm::vec3 finalDT = glm::mix(prevDT, currDT, blendFactor);
            glm::quat finalDR = glm::slerp(prevDR, currDR, blendFactor);
            finalDelta = glm::translate(glm::mat4(1.0f), finalDT) * glm::mat4_cast(finalDR);
        }

        // Apply result to owner object
        Object* owner = GetOwner();
        if (owner)
        {
            glm::vec3 dt; glm::quat dr; glm::vec3 ds; glm::vec3 dskew; glm::vec4 dpers;
            glm::decompose(finalDelta, ds, dr, dt, dskew, dpers);

            glm::vec3 objPos = owner->GetPosition();
            glm::vec3 objRotE = owner->GetRotate3D();
            glm::vec3 objScale = owner->GetSize();

            // Apply translation delta in local space
            // Match the Engine's exact rotation convention (using negative Euler angles internally)
            glm::quat engineRotQuat = glm::quat(glm::radians(-objRotE));

            if (glm::length(dt) > 0.000001f)
            {
                glm::vec3 scaledAndRotatedTranslation = engineRotQuat * (dt * objScale);
                owner->SetPosition(objPos + scaledAndRotatedTranslation);
            }

            // Directly add delta rotation only if there is a noticeable rotation change
            // This prevents glm::eulerAngles from constantly flipping axes in ImGui when dr is identity
            if (glm::abs(dr.w) < 0.999999f || glm::length(glm::vec3(dr.x, dr.y, dr.z)) > 0.000001f)
            {
                glm::quat newEngineRotQuat = engineRotQuat * dr;
                
                // Negate the extracted Euler angles to match the Engine's positive logic
                glm::vec3 newEulerAngles = -glm::degrees(glm::eulerAngles(newEngineRotQuat));

                if (std::isnan(newEulerAngles.x)) newEulerAngles.x = objRotE.x;
                if (std::isnan(newEulerAngles.y)) newEulerAngles.y = objRotE.y;
                if (std::isnan(newEulerAngles.z)) newEulerAngles.z = objRotE.z;

                owner->SetRotate(newEulerAngles);
            }
        }
    }

    // Ensure bone array capacity and fetch encode matrices
    Object* owner = GetOwner();
    glm::mat4 encodeMatrix = glm::mat4(1.0f);
    if (owner)
    {
        DynamicSprite* sprite = owner->GetComponent<DynamicSprite>();
        if (sprite && !sprite->GetSubMeshes().empty())
        {
            auto* meshData = sprite->GetSubMeshes()[0]->GetData<BufferWrapper::DynamicSprite3DMesh>();
            if (meshData)
            {
                if (finalBoneMatrices.size() < meshData->boneInfoMap.size())
                    finalBoneMatrices.resize(meshData->boneInfoMap.size(), glm::mat4(1.0f));
                
                encodeMatrix = meshData->meshNormalizationTransform;
            }
        }
    }

    // Calculate bone hierarchy transforms
    CalculateBoneTransform(&currentAnimation->GetRootNode(), glm::mat4(1.0f), encodeMatrix, false);

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

    lastRootMotionTime = currentTime;
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
    lastRootMotionTime = 0.0f;
    isLooping = isLoop;
    animationSpeed = speed;
    justLooped = false;
    playbackState = PlaybackState::Playing;

    // Auto-detect root bone name (Dynamic Traversal)
    if (enableRootMotion && rootBoneName.empty() && newAnimation)
    {
        std::function<void(const AssimpNodeData&)> findFirstBone;
        findFirstBone = [&](const AssimpNodeData& node) {
            if (!rootBoneName.empty()) return;
            
            // Check if this node exists in the valid bone map
            if (newAnimation->GetBoneIDMap().find(node.originalBoneName) != newAnimation->GetBoneIDMap().end())
            {
                rootBoneName = node.originalBoneName;
                return;
            }
            
            // Recursively check children
            for (const auto& child : node.children)
            {
                findFirstBone(child);
            }
        };

        findFirstBone(newAnimation->GetRootNode());
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
}

glm::mat4 SkeletalAnimator::GetRootTransformAtTime(SkeletalAnimation* anim, float time)
{
    if (!anim || rootBoneName.empty()) return glm::mat4(1.0f);

    std::string posBoneName = rootBoneName + "_$AssimpFbx$_Translation";
    std::string rotBoneName = rootBoneName + "_$AssimpFbx$_Rotation";
    std::string rotBoneNameMocap = rootBoneName + "_$AssMocapFix$_Rotation";

    SkeletalBone* posBone = anim->FindBone(posBoneName);
    SkeletalBone* rotBone = anim->FindBone(rotBoneName);
    if (!rotBone) rotBone = anim->FindBone(rotBoneNameMocap);

    if (!posBone) posBone = anim->FindBone(rootBoneName);
    
    // Fallback for Rotation: If FBX dummy nodes don't exist (e.g., standard glTF export),
    // use the actual root bone itself.
    if (!rotBone) rotBone = anim->FindBone(rootBoneName);

    if (!posBone && !rotBone) return glm::mat4(1.0f);

    glm::vec3 targetPos = posBone ? posBone->GetInterpolatedPosition(time) : glm::vec3(0.f);
    glm::quat targetRot = rotBone ? rotBone->GetInterpolatedRotation(time) : glm::quat(1, 0, 0, 0);

    return glm::translate(glm::mat4(1.0f), targetPos) * glm::mat4_cast(targetRot);
}

glm::mat4 SkeletalAnimator::ExtractRootMotionDelta(SkeletalAnimation* anim, float timeStart, float timeEnd)
{
    glm::mat4 startM = GetRootTransformAtTime(anim, timeStart);
    glm::mat4 endM = GetRootTransformAtTime(anim, timeEnd);

    // Delta in local space of startM
    glm::mat4 delta = glm::inverse(startM) * endM;

    if (bakeOptions.bakePositionX) delta[3][0] = 0.0f;
    if (bakeOptions.bakePositionY) delta[3][1] = 0.0f;
    if (bakeOptions.bakePositionZ) delta[3][2] = 0.0f;
    
    // If we bake rotation, we remove the rotation part from delta 
    // leaving only translation and scale
    if (bakeOptions.bakeRotation)
    {
        delta[0] = glm::vec4(1, 0, 0, 0);
        delta[1] = glm::vec4(0, 1, 0, 0);
        delta[2] = glm::vec4(0, 0, 1, 0);
    }

    return delta;
}

void SkeletalAnimator::UpdateRootMotionTransformToTime(float time)
{
    if (!enableRootMotion || !currentAnimation) return;

    Object* owner = GetOwner();
    if (!owner) return;

    glm::mat4 delta = ExtractRootMotionDelta(currentAnimation, lastRootMotionTime, time);

    glm::vec3 dt; glm::quat dr; glm::vec3 ds; glm::vec3 dskew; glm::vec4 dpers;
    glm::decompose(delta, ds, dr, dt, dskew, dpers);

    glm::vec3 objPos = owner->GetPosition();
    glm::vec3 objRotE = owner->GetRotate3D();
    glm::vec3 objScale = owner->GetSize();

    // Apply translation delta in local space
    // Match the Engine's exact rotation convention (using negative Euler angles internally)
    glm::quat engineRotQuat = glm::quat(glm::radians(-objRotE));

    if (glm::length(dt) > 0.000001f)
    {
        glm::vec3 scaledAndRotatedTranslation = engineRotQuat * (dt * objScale);
        owner->SetPosition(objPos + scaledAndRotatedTranslation);
    }

    // Directly add delta rotation only if there is a noticeable rotation change
    if (glm::abs(dr.w) < 0.999999f || glm::length(glm::vec3(dr.x, dr.y, dr.z)) > 0.000001f)
    {
        glm::quat newEngineRotQuat = engineRotQuat * dr;
        
        // Negate the extracted Euler angles to match the Engine's positive logic
        glm::vec3 newEulerAngles = -glm::degrees(glm::eulerAngles(newEngineRotQuat));

        if (std::isnan(newEulerAngles.x)) newEulerAngles.x = objRotE.x;
        if (std::isnan(newEulerAngles.y)) newEulerAngles.y = objRotE.y;
        if (std::isnan(newEulerAngles.z)) newEulerAngles.z = objRotE.z;

        owner->SetRotate(newEulerAngles);
    }

    lastRootMotionTime = time;
}

void SkeletalAnimator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform, const glm::mat4& encodeMatrix, bool parentIsDummy)
{
    const std::string& nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    if (node->isDummyNode)
    {
        // Translation Dummy: Apply position from animation
        if (node->isTranslationDummy)
        {
            const std::string& boneName = node->originalBoneName;

            SkeletalBone* bone = currentAnimation ? currentAnimation->FindBone(boneName) : nullptr;
            SkeletalBone* prevBone = (blendFactor < 1.0f && previousAnimation)
                ? previousAnimation->FindBone(boneName) : nullptr;

            if (bone)
            {
                glm::vec3 p1 = bone->GetInterpolatedPosition(currentTime);

                if (!prevBone)
                {
                    nodeTransform = node->transformation;
                    nodeTransform[3] = glm::vec4(p1, 1.0f);
                }
                else
                {
                    glm::vec3 p2 = prevBone->GetInterpolatedPosition(previousTime);
                    glm::vec3 p = glm::mix(p2, p1, blendFactor);
                    nodeTransform = node->transformation;
                    nodeTransform[3] = glm::vec4(p, 1.0f);
                }
            }
        }

        // Root Motion: Reset dummy transforms so the mesh stays centered
        // Only reset the axes that are NOT baked into the pose.
        // If baked (true), the animation stats on the bone and moves the mesh.
        // If not baked (false), the animation moves the Object, so we reset the bone to prevent double-movement.
        if (enableRootMotion && node->originalBoneName == rootBoneName)
        {
            if (node->isTranslationDummy)
            {
                if (!bakeOptions.bakePositionX) nodeTransform[3][0] = 0.0f;
                if (!bakeOptions.bakePositionY) nodeTransform[3][1] = 0.0f;
                if (!bakeOptions.bakePositionZ) nodeTransform[3][2] = 0.0f;
            }
            else if (nodeName.find("_Rotation") != std::string::npos)
            {
                if (!bakeOptions.bakeRotation) nodeTransform = glm::mat4(1.0f);
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
            // Extract and remove PreRotation to get pure Local Rotation
            glm::quat preRot = glm::quat(1.f, 0.f, 0.f, 0.f);
            const auto& preRotMap = currentAnimation->GetPreRotations();
            auto preIt = preRotMap.find(nodeName);
            if (preIt != preRotMap.end())
                preRot = preIt->second;

            // Position follows animation for standard rigs, but uses bind pose if an FBX Translation dummy handled it
            glm::vec3 p1 = parentIsDummy ? glm::vec3(node->transformation[3]) : bone->GetInterpolatedPosition(currentTime);

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
                glm::vec3 p2 = parentIsDummy ? glm::vec3(node->transformation[3]) : prevBone->GetInterpolatedPosition(previousTime);
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

    // Send final bone matrices
    if (currentAnimation)
    {
        const auto& boneInfoMap = currentAnimation->GetBoneIDMap();
        auto it = boneInfoMap.find(nodeName);
        if (it != boneInfoMap.end())
        {
            int index = it->second.id;
            if (index >= 0 && index < (int)finalBoneMatrices.size())
            {
                finalBoneMatrices[index] = encodeMatrix * globalTransformation * it->second.offset;
            }
        }
    }

    // Recursively process children
    for (const auto& child : node->children)
        CalculateBoneTransform(&child, globalTransformation, encodeMatrix, node->isDummyNode);
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
    return parent->isDummyNode;
}

float SkeletalAnimator::GetDuration() const 
{ 
    return currentAnimation ? currentAnimation->GetDuration() : 0.0f; 
}