//Author: DOYEONG LEE
//Project: CubeEngine
//File: SpriteManager.cpp
#include "SpriteManager.hpp"
#include "BasicComponents/DynamicSprite.hpp"
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
    Engine::GetLogger().LogDebug(LogCategory::Engine, "Sprite Manager Deleted");
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

void SpriteManager::AddSprite(ISprite* sprite_)
{
    dynamicSprites.push_back(dynamic_cast<DynamicSprite*>(sprite_));
}

void SpriteManager::DeleteSprite(ISprite* sprite_)
{
    if (!dynamicSprites.empty())
    {
        //auto iterator = std::find(sprites.begin(), sprites.end(), sprite_);
        auto iterator = std::ranges::find(dynamicSprites.begin(), dynamicSprites.end(), sprite_);
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
        if (iterator != dynamicSprites.end())
        {
            //delete sprite_;
            dynamicSprites.erase(iterator);
        }
    }
}
