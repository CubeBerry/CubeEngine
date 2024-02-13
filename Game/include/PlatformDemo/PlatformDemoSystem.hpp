//Author: DOYEONG LEE
//Project: CubeEngine
//File: PlatformDemoSystem.hpp
#pragma once

#include <vector>
#include "BasicComponents/Sprite.hpp"

struct TargetWall
{
	Sprite* rect = nullptr;
	glm::vec2 startPos{ 0.f,0.f };
	glm::vec2 endPos{ 0.f,0.f };

	glm::vec2 size{ 0.f,0.f };
	glm::vec2 pos{ 0.f,0.f };
	std::string name = "";
	std::string spriteName = "";
	ObjectType type = ObjectType::NONE;
};

enum class EditorMode
{
	OBJMODIFIER,
	OBJCREATOR,
	WALLCREATOR,
	BACKGCREATOR
};

class PDemoMapEditorDemo
{
public:
	PDemoMapEditorDemo() = default;
	~PDemoMapEditorDemo() {};

	void LoadLevelData(const std::filesystem::path& filePath);
	void SaveLevelData(const std::filesystem::path& outFilePath);

#ifdef _DEBUG
	void Init();
	void Update(float dt);
	void End();
#endif

	bool GetIsEditorMod() { return isEditorMod; }
	void SetIsEditorMod(bool state) { isEditorMod = state; }
private:
	void ObjectCreator();
	void WallCreator();

	bool isEditorMod = false;
	EditorMode mode = EditorMode::WALLCREATOR;
	std::vector<Object*> objects;
	std::vector<Object*> walls;

	TargetWall* target = nullptr;
	bool isWallSetting = false;

	glm::vec2 gridSize = { 32.f, 32.f };
};

class PlatformDemoSystem
{
public:
	PlatformDemoSystem() {}
	~PlatformDemoSystem() = default;

	void Init();
	void Update(float dt);
	void End();

	void LoadLevelData(const std::filesystem::path& filePath) { mapEditor->LoadLevelData(filePath); };
	void SaveLevelData(const std::filesystem::path& outFilePath) { mapEditor->SaveLevelData(outFilePath); };

	void SetIsEditorMod(bool state) { mapEditor->SetIsEditorMod(state); }
	bool GetIsEditorMod() { return mapEditor->GetIsEditorMod(); }
private:
	PDemoMapEditorDemo* mapEditor = nullptr;
};