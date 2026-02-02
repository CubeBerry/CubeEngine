//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalBone.cpp

#define GLM_ENABLE_EXPERIMENTAL

#include "SkeletalAnimation/SkeletalBone.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

#include <assimp/anim.h>

// Initialize bone data and extract keyframes from the Assimp animation channel
SkeletalBone::SkeletalBone(const std::string& name, int ID, const aiNodeAnim* channel)
    : name(name), id(ID), localTransform(1.0f)
{
    // Extract position keyframes and their timestamps
    numPositions = static_cast<int>(channel->mNumPositionKeys);
    for (int i = 0; i < numPositions; ++i) {
        aiVector3D aiPosition = channel->mPositionKeys[i].mValue;
        float timeStamp = channel->mPositionKeys[i].mTime;
        positions.push_back({ glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z), timeStamp });
    }

    // Extract rotation keyframes and convert them to quaternions
    numRotations = static_cast<int>(channel->mNumRotationKeys);
    for (int i = 0; i < numRotations; ++i) {
        aiQuaternion aiOrientation = channel->mRotationKeys[i].mValue;
        float timeStamp = channel->mRotationKeys[i].mTime;
        rotations.push_back({ glm::quat(aiOrientation.w, aiOrientation.x, aiOrientation.y, aiOrientation.z), timeStamp });
    }

    // Extract scaling keyframes and their timestamps
    numScales = static_cast<int>(channel->mNumScalingKeys);
    for (int i = 0; i < numScales; ++i) {
        aiVector3D aiScale = channel->mScalingKeys[i].mValue;
        float timeStamp = channel->mScalingKeys[i].mTime;
        scales.push_back({ glm::vec3(aiScale.x, aiScale.y, aiScale.z), timeStamp });
    }
}

// Update the local transform by combining interpolated translation, rotation, and scale
void SkeletalBone::Update(float animationTime)
{
    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScaling(animationTime);
    localTransform = translation * rotation * scale;
}

// Calculate the interpolated position matrix
glm::mat4 SkeletalBone::InterpolatePosition(float animationTime)
{
    if (numPositions == 1)
        return glm::translate(glm::mat4(1.0f), positions[0].position);

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(positions[p0Index].timeStamp, positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(positions[p0Index].position, positions[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

// Calculate the interpolated rotation matrix using spherical linear interpolation
glm::mat4 SkeletalBone::InterpolateRotation(float animationTime)
{
    if (numRotations == 1) {
        auto rotation = glm::normalize(rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(rotations[p0Index].timeStamp, rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(rotations[p0Index].orientation, rotations[p1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

// Calculate the interpolated scaling matrix
glm::mat4 SkeletalBone::InterpolateScaling(float animationTime)
{
    if (numScales == 1)
        return glm::scale(glm::mat4(1.0f), scales[0].scale);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(scales[p0Index].timeStamp, scales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(scales[p0Index].scale, scales[p1Index].scale, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

// Get the raw interpolated position vector
glm::vec3 SkeletalBone::GetInterpolatedPosition(float animationTime)
{
    if (numPositions == 1)
        return positions[0].position;

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(positions[p0Index].timeStamp, positions[p1Index].timeStamp, animationTime);
    return glm::mix(positions[p0Index].position, positions[p1Index].position, scaleFactor);
}

// Get the raw interpolated rotation quaternion
glm::quat SkeletalBone::GetInterpolatedRotation(float animationTime)
{
    if (numRotations == 1)
        return glm::normalize(rotations[0].orientation);

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(rotations[p0Index].timeStamp, rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(rotations[p0Index].orientation, rotations[p1Index].orientation, scaleFactor);
    return glm::normalize(finalRotation);
}

// Get the raw interpolated scale vector
glm::vec3 SkeletalBone::GetInterpolatedScale(float animationTime)
{
    if (numScales == 1)
        return scales[0].scale;

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(scales[p0Index].timeStamp, scales[p1Index].timeStamp, animationTime);
    return glm::mix(scales[p0Index].scale, scales[p1Index].scale, scaleFactor);
}

// Find the current keyframe index for positions based on animation time
int SkeletalBone::GetPositionIndex(float animationTime)
{
    for (int index = 0; index < numPositions - 1; ++index) {
        if (animationTime < positions[index + 1].timeStamp)
            return index;
    }
    return 0;
}

// Find the current keyframe index for rotations based on animation time
int SkeletalBone::GetRotationIndex(float animationTime)
{
    for (int index = 0; index < numRotations - 1; ++index) {
        if (animationTime < rotations[index + 1].timeStamp)
            return index;
    }
    return 0;
}

// Find the current keyframe index for scaling based on animation time
int SkeletalBone::GetScaleIndex(float animationTime)
{
    for (int index = 0; index < numScales - 1; ++index) {
        if (animationTime < scales[index + 1].timeStamp)
            return index;
    }
    return 0;
}

// Calculate the normalized distance (0.0 to 1.0) between two keyframes
float SkeletalBone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;

    // Prevent division by zero
    if (framesDiff > 0.0f) {
        scaleFactor = midWayLength / framesDiff;
    }
    return scaleFactor;
}