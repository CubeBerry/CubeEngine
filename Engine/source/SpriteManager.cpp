//Author: DOYEONG LEE
//Project: CubeEngine
//File: SpriteManager.cpp
#include "SpriteManager.hpp"
#include "Engine.hpp"

#include <iostream>

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
	auto iterator = std::find(sprites.begin(), sprites.end(), sprite_);
	for (auto it = iterator + 1; it != sprites.end(); it++)
	{
		(*it)->SetMaterialId((*it)->GetMaterialId() - 1);
		Engine::Instance().GetVKRenderManager()->GetMatrices()->at((*it)->GetMaterialId()).isTex = Engine::Instance().GetVKRenderManager()->GetMatrices()->at((*it)->GetMaterialId() + 1).isTex;
		Engine::Instance().GetVKRenderManager()->GetMatrices()->at((*it)->GetMaterialId()).isTexel = Engine::Instance().GetVKRenderManager()->GetMatrices()->at((*it)->GetMaterialId() + 1).isTexel;
		Engine::Instance().GetVKRenderManager()->GetMatrices()->at((*it)->GetMaterialId()).texIndex = Engine::Instance().GetVKRenderManager()->GetMatrices()->at((*it)->GetMaterialId() + 1).texIndex;
	}
	Engine::Instance().GetVKRenderManager()->DeleteWithIndex();
	sprites.erase(iterator);
}
