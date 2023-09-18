#pragma once
#include "GameState.hpp"
#include <vector>

enum GameLevel
{
	SHADERDEMO,
	VERTICESDEMO,
	NONE
};

class GameStateManager
{
public:
	GameStateManager();
	~GameStateManager();

	void LevelInit();
	void Update(float dt);
	void Draw(float dt);
    void End();
	
	void AddLevel(GameState* level);
	void ChangeLevel(GameLevel changeLV);
	void RestartLevel();

	GameLevel GetCurrentLevel() { return currentLevel; }

private:
	GameLevel currentLevel;
	std::vector<GameState*> levelList;
};