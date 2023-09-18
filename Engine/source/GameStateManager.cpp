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
	levelList.at(currentLevel)->Init();
}

void GameStateManager::Update(float dt)
{
	levelList.at(currentLevel)->Update(dt);
}

void GameStateManager::Draw(float dt)
{
	levelList.at(currentLevel)->Draw(dt);
}

void GameStateManager::End()
{
	levelList.clear();
}

void GameStateManager::AddLevel(GameState* level)
{
	levelList.push_back(level);
}

void GameStateManager::ChangeLevel(GameLevel changeLV)
{
	levelList.at(currentLevel)->End();
	currentLevel = changeLV;
	levelList.at(currentLevel)->Init();
}

void GameStateManager::RestartLevel()
{
	levelList.at(currentLevel)->Restart();
}
