//Author: DOYEONG LEE
//Project: CubeEngine
//File: GameStateManager.cpp
#include "GameStateManager.hpp"

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
	levelList.at(static_cast<int>(currentLevel))->Draw(dt);
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
