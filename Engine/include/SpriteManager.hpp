//Author: DOYEONG LEE
//Project: CubeEngine
//File: SpriteManager.hpp
#pragma once
#include "BasicComponents/ISprite.hpp"

#include <vector>

class DynamicSprite;

class SpriteManager
{
public:
    SpriteManager()  = default;
    ~SpriteManager();

	void Update(float dt);
    void End();

    void AddSprite(ISprite* sprite_);
    void DeleteSprite(ISprite* sprite_);

    int GetSpritesAmount() { return static_cast<int>(sprites.size()); }
    std::vector<DynamicSprite*> GetDynamicSprites() { return sprites; }
private:
    std::vector<DynamicSprite*> sprites;
};