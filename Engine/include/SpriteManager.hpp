//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: SpriteManager.hpp
#pragma once
#include "Interface/ISprite.hpp"
#include <vector>

class DynamicSprite;
class StaticSprite;

class SpriteManager
{
public:
	SpriteManager() = default;
    ~SpriteManager();

	void Update(float dt);
    void End();

    void AddDynamicSprite(DynamicSprite* sprite);
    void DeleteDynamicSprite(DynamicSprite* sprite);
    void RegisterStaticSprite();

    //void AllocateStaticUniform(uint32_t spriteIndex, const ThreeDimension::VertexUniform& vertexUniform);

    int GetDynamicSpritesAmount() const { return static_cast<int>(dynamicSprites.size()); }
    std::vector<DynamicSprite*> GetDynamicSprites() { return dynamicSprites; }
    BufferWrapper* GetGlobalStaticBuffer() const { return globalStaticBuffer.get(); }
private:
    std::vector<DynamicSprite*> dynamicSprites;
    SubMesh globalStaticBuffer;
};