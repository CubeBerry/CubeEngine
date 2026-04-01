//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimation.cpp

#include "Engine.hpp"
#include "SkeletalAnimation/SkeletalAnimation.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <cassert>
#include <iostream>
#include <glm/gtx/matrix_decompose.hpp>

SkeletalAnimation::SkeletalAnimation(const std::string& animationPath, BufferWrapper::DynamicSprite3DMesh* meshData)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);

    if (!scene || !scene->mRootNode) return;

    if (scene->mNumAnimations == 0) {
		Engine::GetLogger().LogError(LogCategory::SkelAnimation, "[SkeletalAnimation] No animations found in: " + animationPath);
        return;
    }

    auto anim = scene->mAnimations[0];
    // Setup global inverse transform
    globalInverseTransform = ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation);
    globalInverseTransform = glm::inverse(globalInverseTransform);

    // Load animation timing
    duration = static_cast<float>(anim->mDuration);
    ticksPerSecond = static_cast<float>(anim->mTicksPerSecond);
    if (ticksPerSecond == 0.0f) ticksPerSecond = 25.0f;

    // Load hierarchy and bone data
    ReadHierarchyData(rootNode, scene->mRootNode);
    ReadMissingBones(anim, *meshData, scene);
    CollectPreRotations(scene->mRootNode, scene);
}

SkeletalBone* SkeletalAnimation::FindBone(const std::string& name)
{
    auto iter = boneNameToIndexMap.find(name);
    if (iter != boneNameToIndexMap.end()) {
        return &bones[iter->second];
    }
    return nullptr;
}

void SkeletalAnimation::ReadMissingBones(const aiAnimation* animation, BufferWrapper::DynamicSprite3DMesh& meshData, const aiScene* scene)
{
    int size = animation->mNumChannels;
    auto& modelBoneInfoMap = meshData.boneInfoMap;
    int& boneCount = meshData.boneCount;
    this->boneInfoMap = modelBoneInfoMap;

    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        // Register new bone if not in existing map
        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            ThreeDimension::BoneInfo info;
            info.id = boneCount;
            info.offset = glm::mat4(1.0f);
            boneInfoMap[boneName] = info;
            boneCount++;
        }

        // Cache bind pose local rotation from offset matrix
        if (modelBoneInfoMap.find(boneName) != modelBoneInfoMap.end())
        {
            glm::mat4 offset = modelBoneInfoMap.at(boneName).offset;
            glm::mat4 bindPoseGlobal = glm::inverse(offset);

            glm::vec3 s, t, skew; glm::vec4 persp; glm::quat bindRot;
            glm::decompose(bindPoseGlobal, s, bindRot, t, skew, persp);
            bindPoseLocalRotations[boneName] = glm::normalize(bindRot);

            bones.emplace_back(SkeletalBone(boneName, boneInfoMap[boneName].id, channel, offset));
        }
        else
        {
            bones.emplace_back(SkeletalBone(boneName, boneInfoMap[boneName].id, channel, glm::mat4(1.0f)));
        }
        boneNameToIndexMap[boneName] = static_cast<int>(bones.size()) - 1;
    }
}

void SkeletalAnimation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
    assert(src);
    dest.name = src->mName.data;
    dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);

    // Cache dummy checks
    dest.isDummyNode = (dest.name.find("_$AssimpFbx$_") != std::string::npos);
    if (dest.isDummyNode) {
        dest.isTranslationDummy = (dest.name.find("_$AssimpFbx$_Translation") != std::string::npos);
        dest.isPreRotationDummy = (dest.name.find("_$AssimpFbx$_PreRotation") != std::string::npos);
        size_t pos = dest.name.find("_$AssimpFbx$_");
        if (pos != std::string::npos) {
            dest.originalBoneName = dest.name.substr(0, pos);
        }
    } else {
        dest.originalBoneName = dest.name;
    }

    for (unsigned int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

void SkeletalAnimation::CollectPreRotations(const aiNode* node, const aiScene* scene)
{
    std::string nodeName = node->mName.data;

    // Extract rotation from FBX PreRotation dummy nodes
    if (nodeName.find("_$AssimpFbx$_PreRotation") != std::string::npos)
    {
        std::string boneName = nodeName.substr(0, nodeName.find("_$AssimpFbx$_"));

        glm::mat4 transform = ConvertMatrixToGLMFormat(node->mTransformation);
        glm::vec3 s, t, skew; glm::vec4 persp; glm::quat rot;
        glm::decompose(transform, s, rot, t, skew, persp);
        preRotations[boneName] = glm::normalize(rot);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        CollectPreRotations(node->mChildren[i], scene);
}

glm::mat4 SkeletalAnimation::ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}