//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimation.hpp

#pragma once
#include "BufferWrapper.hpp" // Need access to DynamicSprite3DMesh
#include "SkeletalBone.hpp"
#include <assimp/scene.h>
#include <vector>
#include <unordered_map>

struct AssimpNodeData
{
    glm::mat4 transformation = glm::mat4(1.0f);
    std::string name = "";
    std::vector<AssimpNodeData> children;

    // Cached fields to prevent string search every frame
    bool isDummyNode = false;
    bool isTranslationDummy = false;
    bool isPreRotationDummy = false;
    std::string originalBoneName = "";
};

class SkeletalAnimation
{
public:
    // Constructor now takes BufferWrapper::DynamicSprite3DMesh* instead of Model*
    SkeletalAnimation(const std::string& animationPath, BufferWrapper::DynamicSprite3DMesh* meshData);
    ~SkeletalAnimation() = default;

    SkeletalBone* FindBone(const std::string& name);

    float GetTicksPerSecond() const { return ticksPerSecond; }
    float GetDuration() const { return duration; }
    const AssimpNodeData& GetRootNode() const { return rootNode; }
    const std::map<std::string, ThreeDimension::BoneInfo>& GetBoneIDMap() const { return boneInfoMap; }
    const std::vector<SkeletalBone>& GetBones() const { return bones; }
    const glm::mat4& GetGlobalInverseTransform() const { return globalInverseTransform; }
    const std::unordered_map<std::string, glm::quat>& GetBindPoseLocalRotations() const { return bindPoseLocalRotations; }
    const std::unordered_map<std::string, glm::quat>& GetPreRotations() const { return preRotations; }
private:
    // Helper function signature update
    void ReadMissingBones(const aiAnimation* animation, BufferWrapper::DynamicSprite3DMesh& meshData, const aiScene* scene);
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
    void CollectPreRotations(const aiNode* node, const aiScene* scene);
    glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from); // Implement or assume exists

    float duration;
    float ticksPerSecond;
    std::vector<SkeletalBone> bones;
    AssimpNodeData rootNode;
    glm::mat4 globalInverseTransform;
    // Local copy of bone info map for this animation
    std::map<std::string, ThreeDimension::BoneInfo> boneInfoMap;
    std::unordered_map<std::string, glm::quat> bindPoseLocalRotations;
    std::unordered_map<std::string, glm::quat> preRotations;
    std::unordered_map<std::string, int> boneNameToIndexMap;
};