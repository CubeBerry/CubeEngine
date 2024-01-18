//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: GameStateManager.cpp
#include "GameStateManager.hpp"
#include "Engine.hpp"

GameStateManager::GameStateManager()
{
	currentLevel = GameLevel::SHADERDEMO;
}

GameStateManager::~GameStateManager()
{
	End();
}

void GameStateManager::LevelInit()
{
	levelList.at(static_cast<int>(currentLevel))->Init();
}

void GameStateManager::Update(float dt)
{
	levelList.at(static_cast<int>(currentLevel))->Update(dt);
}

void GameStateManager::Draw(float dt)
{
	VKRenderManager* renderManager = Engine::Instance().GetVKRenderManager();
	renderManager->BeginRender();
#ifdef _DEBUG
	if (!renderManager->GetIsRecreated())
		levelList.at(static_cast<int>(currentLevel))->ImGuiDraw(dt);
#endif
	renderManager->EndRender();
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
