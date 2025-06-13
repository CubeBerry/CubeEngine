//Author: DOYEONG LEE
//Project: CubeEngine
//File: SpriteManager.hpp
#pragma once
#include "BasicComponents/Sprite.hpp"

#include <vector>

class SpriteManager
{
public:
    SpriteManager()  = default;
    ~SpriteManager();

	void Update(float dt);
    void End();

    void AddSprite(Sprite* sprite_);
    void DeleteSprite(Sprite* sprite_);

    int GetSpritesAmount() { return static_cast<int>(sprites.size()); }
    std::vector<Sprite*> GetSprites() { return sprites; }
private:
    std::vector<Sprite*> sprites;
};