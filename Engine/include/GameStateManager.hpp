//Author: DOYEONG LEE
//Project: CubeEngine
//File: GameStateManager.hpp
#pragma once
#include "GameLevelType.hpp"
#include "GameState.hpp"
#include <vector>

enum class State
{
	START,
	LOAD,
	UPDATE,
	UNLOAD,
	SHUTDOWN,
	RESTART,
	CHANGE,
};

class GameStateManager
{
public:
	GameStateManager();
	~GameStateManager();

	void LevelInit(GameLevel currentLevel_);
	void Update(float dt);
	void Draw();
	void DrawWithImGui(float dt);
    void LevelEnd();
	
	void AddLevel(GameState* level);
	void ChangeLevel(GameLevel lev);
	void RestartLevel();

	GameLevel GetCurrentLevel() { return currentLevel; }

	void SetGameState(State state_) { state = state_; }
	State GetGameState() { return state; }

	void UpdateGameLogic(float dt);
	void UpdateDraw(float dt);
	void StateChanger();
private:
	const char* GameLevelTypeEnumToChar(GameLevel type);
	void LevelInit();
	void CollideObjects();

	GameLevel currentLevel = GameLevel::NONE;
	GameLevel levelSelected = GameLevel::NONE;
	std::vector<GameState*> levelList;
	State state = State::START;

	bool showDescription = false;
};