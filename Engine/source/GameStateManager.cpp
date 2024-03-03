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
}

GameStateManager::~GameStateManager()
{
	for (auto& lev : levelList)
	{
		delete lev;
	}
	levelList.clear();
}

void GameStateManager::LevelInit()
{
	levelList.at(static_cast<int>(currentLevel))->Init();

	if (state != State::CHANGE)
		levelSelected = currentLevel;
}

void GameStateManager::LevelInit(GameLevel currentLevel_)
{
	currentLevel = currentLevel_;
	LevelInit();
	state = State::UPDATE;
#ifdef _DEBUG
	std::cout << "Load Complete" << std::endl;
#endif
}

void GameStateManager::Update(float dt)
{
	switch (state)
	{
	case State::START:
		if (levelList.empty())
		{
			state = State::SHUTDOWN;
		}
		else
		{
			state = State::LOAD;
		}
		break;
	case State::LOAD:
		LevelInit();
#ifdef _DEBUG
		std::cout << "Load Complete" << std::endl;
#endif
		state = State::UPDATE;
#ifdef _DEBUG
		std::cout << "Upadte" << std::endl;
#endif
		break;
	case State::UPDATE:
		if (levelSelected != currentLevel)
		{
			state = State::CHANGE;
		}
		else
		{
			levelList.at(static_cast<int>(currentLevel))->Update(dt);
			Engine::GetSpriteManager().Update(dt);
			Engine::GetObjectManager().Update(dt);
			Engine::GetParticleManager().Update(dt);
			Engine::GetCameraManager().Update();
			CollideObjects();

			if (!(SDL_GetWindowFlags(Engine::GetWindow().GetWindow()) & SDL_WINDOW_MINIMIZED))
			{
#ifdef _DEBUG
				DrawWithImGui(dt);
#else
				Draw();
#endif
			}
		}
		break;
	case State::CHANGE:
		levelList.at(static_cast<int>(currentLevel))->End();
		currentLevel = levelSelected;
#ifdef _DEBUG
		std::cout << "Level Change" << std::endl;
#endif
		state = State::LOAD;
		break;
	case State::RESTART:
		LevelEnd();
		state = State::LOAD;
#ifdef _DEBUG
		std::cout << "Level Restart" << std::endl;
#endif
		break;
	case State::UNLOAD:
		LevelEnd();
		state = State::SHUTDOWN;
#ifdef _DEBUG
		std::cout << "ShutDown" << std::endl;
#endif
		break;
	case State::SHUTDOWN:
		break;
	}
}

void GameStateManager::Draw()
{
	VKRenderManager* renderManager = &Engine::Instance().GetVKRenderManager();
	//if (renderManager->GetMatrices()->size() > 0)
	//{
	renderManager->BeginRender();
	if (!renderManager->GetIsRecreated())
	{
		renderManager->EndRender();
	}
	//}
}

void GameStateManager::LevelEnd()
{
	levelList.at(static_cast<int>(currentLevel))->End();
}

void GameStateManager::AddLevel(GameState* level)
{
	levelList.push_back(level);
}

void GameStateManager::ChangeLevel(GameLevel lev)
{
	levelSelected = lev;
}

void GameStateManager::RestartLevel()
{
	levelList.at(static_cast<int>(currentLevel))->Restart();
}

#ifdef _DEBUG
void GameStateManager::StateChanger()
{
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
}

void GameStateManager::DrawWithImGui(float dt)
{
	VKRenderManager* renderManager = &Engine::Instance().GetVKRenderManager();

	renderManager->BeginRender();
	if (!renderManager->GetIsRecreated())
	{
		levelList.at(static_cast<int>(currentLevel))->ImGuiDraw(dt);
		renderManager->EndRender();
	}
}

const char* GameStateManager::GameLevelTypeEnumToChar(GameLevel type)
{
	switch (type)
	{
		//case GameLevel::VERTICESDEMO:
		//	return "VERTICESDEMO";
		//	break;
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

void GameStateManager::CollideObjects()
{
	for (auto& target : Engine::GetObjectManager().GetObjectMap())
	{
		for (auto& object : Engine::GetObjectManager().GetObjectMap())
		{
			if (target.second != nullptr && object.second != nullptr && target.second != object.second
				&& target.second->HasComponent<Physics2D>() == true && object.second->HasComponent<Physics2D>() == true)
			{
				target.second->CollideObject(object.second.get());
			}
		}
	}
}
