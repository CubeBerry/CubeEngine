#pragma once

#include "BasicComponents/Sprite.hpp"
#include <string>
#include <map>
#include <vector>

enum BackgroundType
{
	NORMAL,
	VPARALLEX,
	NONE
};

struct Background
{
	std::string spriteName = "none";
	glm::vec2 position { 0.f,0.f };
	glm::vec2 speed{ 0.f,0.f };

	glm::vec2 size { 0.f,0.f };
	glm::vec2 sizeIncrements{ 0.f,0.f };
	float angle = 0.f;
	float depth = -1.f;
	BackgroundType type = BackgroundType::NORMAL;

	bool isDeleteWhenOut = false;
	bool isScrolled = true;
	bool isAnimation = false;
	size_t towerFloor = 0;

	int towerHeight = 0;
	Sprite* sprite;
};

class BackgroundManager
{
public:
	BackgroundManager() = default;
	~BackgroundManager() { Clear(); }

	void AddNormalBackground(std::string spriteName_, glm::vec2 position_, glm::vec2 size_, float angle_ = 0.f, glm::vec2 speed_ = { 0.f,0.f }, glm::vec2 sizeIncrements_ = { 0.f,0.f }, float depth_ = -1.f, bool isScrolled_ = true, bool isAnimated = false);

	void AddVerticalParallexBackground(std::string spriteName_, std::string groupName, glm::vec2 position_, glm::vec2 size_, float angle_ = 0.f, float speedY_ = 0.f, float depth_ = -1.f, bool isAnimated = false);
	void AddHorizonParallexBackground(std::string spriteName_, std::string groupName, glm::vec2 position_, glm::vec2 size_, float angle_ = 0.f, float speedX_ = 0.f, float depth_ = -1.f, bool isAnimated = false);
	void AddSaveBackgroundList(std::string spriteName_, std::string groupName, BackgroundType type_, glm::vec2 position_, glm::vec2 size_, float angle_ = 0.f, glm::vec2 speed_ = { 0.f,0.f }, glm::vec2 sizeIncrements_ = { 0.f,0.f }, float depth_ = -1.f, bool isScrolled_ = true, bool isAnimated = false);

	void Update(float dt);

	void SetEditorMod(bool state) { isEditorMod = state; }
	void Clear();

	std::vector<Background> GetNormalBackgroundList() { return normalBackgroundList; }
	std::map<std::string, std::vector<Background>>  GetVerticalParallaxBackgroundList() { return verticalParallaxBackgroundList; }

	std::map<std::string, std::vector<Background>>& GetSaveBackgroundList() { return saveBackgroundList; }
private:
	std::vector<Background> normalBackgroundList;
	std::map<std::string, std::vector<Background>> verticalParallaxBackgroundList;

	std::map<std::string, std::vector<Background>> saveBackgroundList;

	bool isEditorMod = false;
};