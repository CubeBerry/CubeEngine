//Author: DOYEONG LEE
//Project: CubeEngine
//File: SpriteManager.cpp
#include "SpriteManager.hpp"
#include "Engine.hpp"

#include <iostream>

SpriteManager::~SpriteManager()
{
}

void SpriteManager::Update(float dt)
{
	//for (int i = 0; i < sprites.size(); i++)
	//{
	//	//sprites[i]->Update(dt);
	//	sprites[i]->Update(dt, sprites[i]->GetMaterialId());
	//}
}

void SpriteManager::End()
{
}

void SpriteManager::AddSprite(Sprite* sprite_)
{
	sprites.push_back(sprite_);
}

void SpriteManager::DeleteSprite(Sprite* sprite_)
{
	if (sprites.empty() != true)
	{
		auto iterator = std::find(sprites.begin(), sprites.end(), sprite_);
		for (auto it = iterator + 1; it != sprites.end(); it++)
		{
			(*it)->SetMaterialId((*it)->GetMaterialId() - 1);
			Engine::Instance().GetVKRenderManager()->GetMatrices()->at((*it)->GetMaterialId()) = Engine::Instance().GetVKRenderManager()->GetMatrices()->at((*it)->GetMaterialId() + 1);
		}
		Engine::Instance().GetVKRenderManager()->DeleteWithIndex();
		sprites.erase(iterator);
	}
}