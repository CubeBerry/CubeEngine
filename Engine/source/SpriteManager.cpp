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
	Engine::Instance().GetVKRenderManager()->DeleteWithIndex();

	std::vector<UniformMatrix> tempMatrices = *Engine::Instance().GetVKRenderManager()->GetMatrices();
	auto iterator = std::find(sprites.begin(), sprites.end(), sprite_);
	for (auto it = iterator + 1; it != sprites.end(); it++)
	{
		(*it)->SetMaterialId((*it)->GetMaterialId() - 1);
		tempMatrices.at((*it)->GetMaterialId()).isTex = tempMatrices.at((*it)->GetMaterialId() + 1).isTex;
		tempMatrices.at((*it)->GetMaterialId()).isTexel = tempMatrices.at((*it)->GetMaterialId() + 1).isTexel;
		tempMatrices.at((*it)->GetMaterialId()).texIndex = tempMatrices.at((*it)->GetMaterialId() + 1).texIndex;
	}
	sprites.erase(iterator);
	*Engine::Instance().GetVKRenderManager()->GetMatrices() = tempMatrices;
}
