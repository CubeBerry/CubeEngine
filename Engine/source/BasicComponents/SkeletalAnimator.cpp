//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimator.cpp

// Define NOMINMAX to prevent macro collisions with std::min/std::max on Windows
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

// Helper: Reconstruct Model Matrix from Object's Transform Data
glm::mat4 GetObjectModelMatrix(Object* obj)
{
    if (!obj) return glm::mat4(1.0f);

    glm::vec3 pos = obj->GetPosition();
    glm::vec3 rot = obj->GetRotate3D(); // Euler angles
    glm::vec3 scale = obj->GetSize();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);

    // Rotate (Order: Z -> Y -> X or similar, matching Engine convention)
    model = glm::rotate(model, glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::scale(model, scale);

    return model;
}

SkeletalAnimator::SkeletalAnimator()
    : IComponent(ComponentTypes::SKETANIMATOR),
    currentTime(0.0f),
    currentAnimation(nullptr),
    previousAnimation(nullptr)
{
    // Reserve space for bone matrices
    finalBoneMatrices.reserve(ThreeDimension::MAX_BONES);
    for (int i = 0; i < ThreeDimension::MAX_BONES; i++)
    {
        finalBoneMatrices.push_back(glm::mat4(1.0f));
    }
}

void SkeletalAnimator::Init()
{
}

void SkeletalAnimator::End()
{
}

void SkeletalAnimator::Update(float dt)
{
    if (!currentAnimation) return;

    // 1. Handle Blending
    if (currentAnimation)
    {
        if (currentBlendDuration <= 0.0f) currentBlendDuration = 0.01f;
        blendFactor += dt / currentBlendDuration;
        if (blendFactor >= 1.0f)
        {
            blendFactor = 1.0f;
            previousAnimation = nullptr;
        }
    }

    // 2. Update Animation Time
    if (playbackState == PlaybackState::Playing)
    {
        if (previousAnimation && blendFactor < 1.0f)
        {
            previousTime += dt * animationSpeed * previousAnimation->GetTicksPerSecond();
            float prevDuration = previousAnimation->GetDuration();
            if (isLooping)
                previousTime = fmod(previousTime, prevDuration);
            else if (previousTime >= prevDuration)
                previousTime = prevDuration;
        }

        if (!isScrubbing)
        {
            float ticks = dt * animationSpeed * currentAnimation->GetTicksPerSecond();
            currentTime += ticks;

            if (enableRootMotion)
            {
                glm::mat4 rootMotionDelta = CalculateAbsoluteRootMotion(currentAnimation, currentTime, glm::mat4(1.0f));
            }
        }

        float duration = currentAnimation->GetDuration();
        if (currentTime >= duration)
        {
            if (isLooping)
            {
                currentTime = fmod(currentTime, duration);
                justLooped = true;
            }
            else
            {
                currentTime = duration;
            }
        }
    }

    // 3. Fetch the model's globalInverseTransform from meshData (once)
    if (modelGlobalInverseTransform == glm::mat4(1.0f))
    {
        Object* owner = GetOwner();
        if (owner)
        {
            DynamicSprite* sprite = owner->GetComponent<DynamicSprite>();
            if (sprite && !sprite->GetSubMeshes().empty())
            {
                auto* meshData = sprite->GetSubMeshes()[0]->GetData<BufferWrapper::DynamicSprite3DMesh>();
                if (meshData)
                {
                    modelGlobalInverseTransform = meshData->modelGlobalInverseTransform;
                }
            }
        }
    }

    // 4. Calculate Bone Transforms
    if (currentAnimation)
    {
        CalculateBoneTransform(&currentAnimation->GetRootNode(), glm::mat4(1.0f));
    }

    // 5. Upload Matrices to DynamicSprite (GPU)
    Object* owner = GetOwner();
    if (owner)
    {
        DynamicSprite* sprite = owner->GetComponent<DynamicSprite>();
        if (sprite)
        {
            auto& subMeshes = sprite->GetSubMeshes();
            for (auto& subMesh : subMeshes)
            {
                auto* meshData = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>();
                if (meshData)
                {
                    int count = (std::min)((int)finalBoneMatrices.size(), ThreeDimension::MAX_BONES);
                    for (int i = 0; i < count; ++i)
                    {
                        meshData->vertexUniform.finalBones[i] = finalBoneMatrices[i];
                    }

                    if (std::holds_alternative<std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>>(meshData->vertexUniformBuffer))
                    {
                        auto& glBuffer = std::get<std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>>(meshData->vertexUniformBuffer);
                        glBuffer->UpdateUniform(sizeof(ThreeDimension::VertexUniform), &meshData->vertexUniform);
                    }
                }
            }
        }
    }
}

void SkeletalAnimator::PlayAnimation(SkeletalAnimation* newAnimation, bool isLoop, float speed, float blendDuration)
{
    if (currentAnimation == newAnimation) return;

    // Set Blending
    previousAnimation = currentAnimation;
    previousTime = currentTime;
    blendFactor = 0.0f;
    currentBlendDuration = blendDuration;

    // Set New Animation
    currentAnimation = newAnimation;
    currentTime = 0.0f;
    isLooping = isLoop;
    animationSpeed = speed;
    playbackState = PlaybackState::Playing;

    if (!previousAnimation)
    {
        blendFactor = 1.0f;
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

void SkeletalAnimator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation; // Default Local Transform (Bind Pose)

    // 1. Identify Root Bone
    // Root bones handle translation (Root Motion), while others usually only rotate.
    bool isRootBone = (nodeName.find("Hips") != std::string::npos ||
        nodeName.find("Pelvis") != std::string::npos ||
        nodeName.find("Root") != std::string::npos ||
        nodeName == rootBoneName);

    // 2. Get Current Animation Transform
    SkeletalBone* bone = currentAnimation->FindBone(nodeName);
    if (bone)
    {
        bone->Update(currentTime);

        if (isRootBone)
        {
            // Apply full animation transform (Translation, Rotation, Scale) for Root
            nodeTransform = bone->GetLocalTransform();
        }
        else
        {
            // For other bones: Keep original Bind Pose translation (bone length)
            // and apply only animation Rotation and Scale.
            glm::vec3 origTranslation = glm::vec3(node->transformation[3]);
            glm::quat animRot = bone->GetInterpolatedRotation(currentTime);
            glm::vec3 animScale = bone->GetInterpolatedScale(currentTime);

            // Rebuild the Local Transform matrix
            nodeTransform = glm::translate(glm::mat4(1.0f), origTranslation) *
                glm::mat4_cast(animRot) *
                glm::scale(glm::mat4(1.0f), animScale);
        }
    }

    // 3. Handle Animation Blending
    if (blendFactor < 1.0f && previousAnimation)
    {
        SkeletalBone* prevBone = previousAnimation->FindBone(nodeName);
        if (prevBone)
        {
            prevBone->Update(previousTime);
            glm::mat4 prevTransform;

            // Apply the same translation logic to the previous animation
            if (isRootBone)
            {
                prevTransform = prevBone->GetLocalTransform();
            }
            else
            {
                glm::vec3 origTranslation = glm::vec3(node->transformation[3]);
                glm::quat animRot = prevBone->GetInterpolatedRotation(previousTime);
                glm::vec3 animScale = prevBone->GetInterpolatedScale(previousTime);

                prevTransform = glm::translate(glm::mat4(1.0f), origTranslation) *
                    glm::mat4_cast(animRot) *
                    glm::scale(glm::mat4(1.0f), animScale);
            }

            // Interpolate between previous and current animation
            glm::vec3 p1, p2, p;
            glm::quat r1, r2, r;
            glm::vec3 s1, s2, s;
            glm::vec3 skew;
            glm::vec4 perspective;

            glm::decompose(nodeTransform, s1, r1, p1, skew, perspective);
            glm::decompose(prevTransform, s2, r2, p2, skew, perspective);

            p = glm::mix(p2, p1, blendFactor);
            r = glm::slerp(r2, r1, blendFactor);
            s = glm::mix(s2, s1, blendFactor);

            // Recompose the final blended matrix
            nodeTransform = glm::translate(glm::mat4(1.0f), p) *
                glm::mat4_cast(r) *
                glm::scale(glm::mat4(1.0f), s);
        }
    }

    // 4. Calculate Global Transform
    glm::mat4 globalTransformation = parentTransform * nodeTransform;
    globalBoneTransforms[nodeName] = globalTransformation;

    // 5. Update Final Bone Matrix for GPU Skinning
    const auto& boneInfoMap = currentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap.at(nodeName).id;
        glm::mat4 offset = boneInfoMap.at(nodeName).offset;
        if (index < finalBoneMatrices.size())
        {
            // Final Matrix = Global Pose * Inverse Bind Pose (Offset)
            finalBoneMatrices[index] = globalTransformation * offset;
        }
    }

    // 6. Recursively process children nodes
    for (const auto& child : node->children)
    {
        CalculateBoneTransform(&child, globalTransformation);
    }
}

const std::vector<glm::mat4>& SkeletalAnimator::GetFinalBoneMatrices() const
{
    return finalBoneMatrices;
}

float SkeletalAnimator::GetCurrentAnimationTime() const
{
    return currentTime;
}

float SkeletalAnimator::GetDuration() const
{
    return currentAnimation ? currentAnimation->GetDuration() : 0.0f;
}

void SkeletalAnimator::SetLooping(bool loop)
{
    isLooping = loop;
}

void SkeletalAnimator::SetCurrentAnimationTime(float time)
{
    currentTime = time;
    justLooped = false;
}

void SkeletalAnimator::UpdateRootMotionTransformToTime(float time)
{
    Object* owner = GetOwner();
    if (!owner || !currentAnimation) return;

    // Calculate delta from start to time
    glm::mat4 rootMotion = CalculateAbsoluteRootMotion(currentAnimation, time, glm::mat4(1.0f));

    // Extract position delta
    glm::vec3 deltaPos = glm::vec3(rootMotion[3]);

    // Apply to Object (Simplified: Add delta to current position)
    // In a real editor scrubbing scenario, you might want to set absolute position relative to start
    glm::vec3 currentPos = owner->GetPosition();
    owner->SetPosition(currentPos + deltaPos);
}

glm::mat4 SkeletalAnimator::CalculateAbsoluteRootMotion(SkeletalAnimation* anim, float time, glm::mat4 startTransform)
{
    if (!anim) return glm::mat4(1.0f);
    if (rootBoneName.empty()) return startTransform;

    // Define special bone names for FBX/Mocap
    std::string posBoneName_Fbx = rootBoneName + "_$AssimpFbx$_Translation";
    std::string rotBoneName_Fbx = rootBoneName + "_$AssimpFbx$_Rotation";
    std::string rotBoneName_Mocap = rootBoneName + "_$AssMocapFix$_Rotation";

    SkeletalBone* posBone = anim->FindBone(posBoneName_Fbx);
    SkeletalBone* rotBone = anim->FindBone(rotBoneName_Fbx);

    if (!rotBone)
    {
        rotBone = anim->FindBone(rotBoneName_Mocap);
    }

    // Calculate Delta
    glm::mat4 totalDelta = glm::mat4(1.0f);

    if (posBone && rotBone)
    {
        // Calculate transforms at time 0 and current time
        glm::mat4 startBoneLocalT = glm::translate(glm::mat4(1.0f), posBone->GetInterpolatedPosition(0.0f)) * glm::mat4_cast(rotBone->GetInterpolatedRotation(0.0f));
        glm::mat4 targetBoneLocalT = glm::translate(glm::mat4(1.0f), posBone->GetInterpolatedPosition(time)) * glm::mat4_cast(rotBone->GetInterpolatedRotation(time));

        totalDelta = glm::inverse(startBoneLocalT) * targetBoneLocalT;

        // Apply to Object
        Object* owner = GetOwner();
        if (owner && enableRootMotion)
        {
            // Create Rotation Matrix from Object's Euler Angles
            glm::mat4 rotationMatrix = glm::mat4(1.0f);
            glm::vec3 rot = owner->GetRotate3D();
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

            // Extract translation from delta and rotate to world space
            glm::vec3 deltaPos = glm::vec3(totalDelta[3]);
            glm::vec3 worldDelta = glm::vec3(rotationMatrix * glm::vec4(deltaPos, 1.0f));

            // Update Object Position
            glm::vec3 currentPos = owner->GetPosition();
            owner->SetPosition(currentPos + worldDelta);
        }

        return startTransform * totalDelta;
    }

    // Fallback: Standard Root Bone
    SkeletalBone* rootBone = anim->FindBone(rootBoneName);
    if (rootBone)
    {
        // Simple implementation for standard bone if needed
        // For now, returning identity if special bones aren't found
    }

    return glm::mat4(1.0f);
}