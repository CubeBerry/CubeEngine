//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimation.cpp

#include "SkeletalAnimation/SkeletalAnimation.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h> // aiProcess_Triangulate
#include <cassert>
#include <iostream>

// Constructor that loads animation data from a file using Assimp
SkeletalAnimation::SkeletalAnimation(const std::string& animationPath, BufferWrapper::DynamicSprite3DMesh* meshData)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);

    // Check if the scene was loaded successfully
    if (!scene || !scene->mRootNode) {
        return;
    }

    // Check if scene has animations
    if (scene->mNumAnimations == 0) {
        std::cerr << "[SkeletalAnimation] No animations found in: " << animationPath << std::endl;
        return;
    }

    globalInverseTransform = ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation);
    globalInverseTransform = glm::inverse(globalInverseTransform);

    // Get the first animation sequence and its timing info
    auto animation = scene->mAnimations[0];
    duration = static_cast<float>(animation->mDuration);
    ticksPerSecond = static_cast<float>(animation->mTicksPerSecond);

    // Provide a default fallback for ticks per second if not specified
    if (ticksPerSecond == 0.0f) ticksPerSecond = 25.0f;

    // Recursively build the node hierarchy from the scene root
    ReadHierarchyData(rootNode, scene->mRootNode);

    // Map animation channels to the mesh's bone structure
    ReadMissingBones(animation, *meshData, scene);
}

// Locate a specific bone object within the animation's bone list
SkeletalBone* SkeletalAnimation::FindBone(const std::string& name)
{
    auto iter = std::find_if(bones.begin(), bones.end(),
        [&](const SkeletalBone& bone) { return bone.GetBoneName() == name; }
    );
    if (iter == bones.end()) return nullptr;
    else return &(*iter);
}

// Link animation channels to bone information and handle missing bones
void SkeletalAnimation::ReadMissingBones(const aiAnimation* animation, BufferWrapper::DynamicSprite3DMesh& meshData, const aiScene* scene)
{
    int size = animation->mNumChannels;

    auto& modelBoneInfoMap = meshData.boneInfoMap;
    int& boneCount = meshData.boneCount;

    // 1. Copy existing bone info map
    this->boneInfoMap = modelBoneInfoMap;

    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        // 2. If bone is not in the map, register it
        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            ThreeDimension::BoneInfo info;
            info.id = boneCount;
            info.offset = glm::mat4(1.0f);

            // Try to find the offset matrix from the scene's meshes
            if (scene && scene->HasMeshes())
            {
                for (unsigned int m = 0; m < scene->mNumMeshes; m++)
                {
                    const aiMesh* mesh = scene->mMeshes[m];
                    for (unsigned int b = 0; b < mesh->mNumBones; b++)
                    {
                        std::string meshBoneName = mesh->mBones[b]->mName.data;
                        if (meshBoneName == boneName)
                        {
                            info.offset = ConvertMatrixToGLMFormat(mesh->mBones[b]->mOffsetMatrix);
                            break;
                        }
                    }
                }
            }
            boneInfoMap[boneName] = info;
            boneCount++;
        }
        bones.emplace_back(SkeletalBone(boneName, boneInfoMap[boneName].id, channel));
    }
}

// Recursively copy Assimp's node structure into our internal hierarchy format
void SkeletalAnimation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
    assert(src);
    dest.name = src->mName.data;
    dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);

    // Process all children of the current node
    for (unsigned int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

// Convert Assimp's row-major matrix to GLM's column-major matrix format
glm::mat4 SkeletalAnimation::ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}