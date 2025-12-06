//Author: DOYEONG LEE
//Project: CubeEngine
//File: SpriteManager.hpp
#pragma once
#include "BasicComponents/StaticSprite.hpp"

#include <vector>

class DynamicSprite;

class SpriteManager
{
public:
    SpriteManager() { staticSprite = std::make_unique<StaticSprite>(); }
    ~SpriteManager();

	void Update(float dt);
    void End();

    void AddSprite(ISprite* sprite_);
    void DeleteSprite(ISprite* sprite_);

    int GetDynamicSpritesAmount() { return static_cast<int>(dynamicSprites.size()); }
    std::vector<DynamicSprite*> GetDynamicSprites() { return dynamicSprites; }
    StaticSprite* GetStaticSprite() { return staticSprite.get(); }
private:
    std::vector<DynamicSprite*> dynamicSprites;
    std::unique_ptr<StaticSprite> staticSprite;
};