//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: GameStateManager.cpp
#include "GameStateManager.hpp"
#include "Engine.hpp"
#ifdef _DEBUG
#include "imgui.h"
#endif

GameStateManager::GameStateManager()
{
	currentLevel = GameLevel::VERTICESDEMO;
}

GameStateManager::~GameStateManager()
{
	End();
}

void GameStateManager::LevelInit()
{
	levelList.at(static_cast<int>(currentLevel))->Init();
}

void GameStateManager::LevelInit(GameLevel currentLevel_)
{
	currentLevel = currentLevel_; 
	LevelInit();
}

void GameStateManager::Update(float dt)
{
	levelList.at(static_cast<int>(currentLevel))->Update(dt);
}

void GameStateManager::Draw(float dt)
{
	VKRenderManager* renderManager = Engine::Instance().GetVKRenderManager();
	//if (renderManager->GetMatrices()->size() > 0)
	//{
		renderManager->BeginRender();
		if (!renderManager->GetIsRecreated())
		{
#ifdef _DEBUG
			levelList.at(static_cast<int>(currentLevel))->ImGuiDraw(dt);
#endif
			renderManager->EndRender();
		}
	//}
#ifdef _DEBUG
		if (levelSelected != currentLevel)
		{
			ChangeLevel(levelSelected);
		}
#endif

}

void GameStateManager::End()
{
	levelList.at(static_cast<int>(currentLevel))->End();
	levelList.clear();
}

void GameStateManager::AddLevel(GameState* level)
{
	levelList.push_back(level);
}

void GameStateManager::ChangeLevel(GameLevel changeLV)
{
	levelList.at(static_cast<int>(currentLevel))->End();
	currentLevel = changeLV;
	levelList.at(static_cast<int>(currentLevel))->Init();
}

void GameStateManager::RestartLevel()
{
	levelList.at(static_cast<int>(currentLevel))->Restart();
}

#ifdef _DEBUG
void GameStateManager::StateChanger()
{
	levelSelected = currentLevel;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Change Level"))
        {
			for (int i = 0; i < levelList.size(); i++)
			{
				std::string levelName = GameLevelTypeEnumToChar(static_cast<GameLevel>(i));
				if (ImGui::MenuItem(levelName.c_str(), std::to_string(i).c_str(), levelSelected == static_cast<GameLevel>(i)))
				{
					levelSelected = static_cast<GameLevel>(i);
				}
			}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

	//if (levelSelected != currentLevel)
	//{
	//	ChangeLevel(levelSelected);
	//}
}

const char* GameStateManager::GameLevelTypeEnumToChar(GameLevel type)
{
	switch (type)
	{
	case GameLevel::VERTICESDEMO:
		return "VERTICESDEMO";
		break;
	case GameLevel::POCKETBALL:
		return "POCKETBALL";
		break;
	case GameLevel::PLATFORMDEMO:
		return "PLATFORMDEMO";
		break;
	}
	return "NONE";
}
#endif
