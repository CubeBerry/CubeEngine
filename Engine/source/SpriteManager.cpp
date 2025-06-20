//Author: DOYEONG LEE
//Project: CubeEngine
//File: SpriteManager.cpp
#include "SpriteManager.hpp"
#include "Engine.hpp"

SpriteManager::~SpriteManager()
{
	//if (sprites.empty() != true)
	//{
	//	auto iterator = sprites.begin();
	//	for (auto it = iterator; it != sprites.end(); ++it)
	//	{
	//		//Engine::Instance().GetVKRenderManager().DeleteWithIndex();
	//		sprites.erase(iterator);
	//		std::cout << sprites.size() << '\n'';
	//	}
	//}
}

void SpriteManager::Update(float /*dt*/)
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
    if (!sprites.empty())
    {
        //auto iterator = std::find(sprites.begin(), sprites.end(), sprite_);
        auto iterator = std::ranges::find(sprites.begin(), sprites.end(), sprite_);
        //for (auto it = iterator + 1; it != sprites.end(); ++it)
        //{
            //(*it)->SetMaterialId((*it)->GetMaterialId() - 1);

            //switch(Engine::Instance().GetRenderManager()->GetRenderType())
            //{
            //case RenderType::TwoDimension:
            //    Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at((*it)->GetMaterialId() + 1);
            //    Engine::Instance().GetRenderManager()->GetFragmentUniforms2D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetFragmentUniforms2D()->at((*it)->GetMaterialId() + 1);
            //    break;
            //case RenderType::ThreeDimension:
            //    Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at((*it)->GetMaterialId() + 1);
            //    Engine::Instance().GetRenderManager()->GetFragmentUniforms3D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetFragmentUniforms3D()->at((*it)->GetMaterialId() + 1);
            //    Engine::Instance().GetRenderManager()->GetMaterialUniforms3D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetMaterialUniforms3D()->at((*it)->GetMaterialId() + 1);
            //    break;
            //}
        //}
        //Engine::Instance().GetRenderManager()->DeleteWithIndex(0);
        if (iterator != sprites.end())
        {
            //delete sprite_;
            sprites.erase(iterator);
        }
    }
}
