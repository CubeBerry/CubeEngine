//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalBone.hpp
#pragma once

#include <vector>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct aiNodeAnim;

// Keyframe data structures
struct KeyPosition { glm::vec3 position; float timeStamp; };
struct KeyRotation { glm::quat orientation; float timeStamp; };
struct KeyScale { glm::vec3 scale; float timeStamp; };

// Manages all keyframe information for a single bone
class SkeletalBone
{
public:
    SkeletalBone(const std::string& name, int ID, const aiNodeAnim* channel);

    void Update(float animationTime);

    glm::mat4 GetLocalTransform() const { return localTransform; }
    const std::string& GetBoneName() const { return name; }
    int GetBoneID() const { return id; }

    glm::vec3 GetInterpolatedPosition(float animationTime);
    glm::quat GetInterpolatedRotation(float animationTime);
    glm::vec3 GetInterpolatedScale(float animationTime);

private:
    glm::mat4 InterpolatePosition(float animationTime);
    glm::mat4 InterpolateRotation(float animationTime);
    glm::mat4 InterpolateScaling(float animationTime);

    int GetPositionIndex(float animationTime);
    int GetRotationIndex(float animationTime);
    int GetScaleIndex(float animationTime);
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;
    int numPositions;
    int numRotations;
    int numScales;

    glm::mat4 localTransform;
    std::string name;
    int id;
};