//Author: DOYEONG LEE
//Project: CubeEngine
//File: SpriteManager.hpp
#pragma once
#include "BasicComponents/Sprite.hpp"

#include <vector>
#include <iostream>

class SpriteManager
{
public:
    SpriteManager()  = default;
    ~SpriteManager() = default;

	void Update(float dt);
    void End();

    void AddSprite(Sprite* sprite_);
    void DeleteSprite(Sprite* sprite_);
private:
    std::vector<Sprite*>  sprites;
};