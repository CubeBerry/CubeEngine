#include "Background.hpp"
#include "GameState.hpp"
#include "Engine.hpp"

bool isInOfCamera(Background& back)
{
	glm::vec2 windowSize = { static_cast<float>(Engine::GetCameraManager().GetViewSize().x),
		static_cast<float>(Engine::GetCameraManager().GetViewSize().y) };
	windowSize = (windowSize / Engine::GetCameraManager().GetZoom());
	glm::vec2 cameraCenter = Engine::GetCameraManager().GetCenter();
	if ((back.position.x - (back.size.x)) < (windowSize.x / 2.f + cameraCenter.x) && (back.position.x + (back.size.x)) > -(windowSize.x / 2.f - cameraCenter.x)
		&& (back.position.y - (back.size.y)) < (windowSize.y / 2.f + cameraCenter.y) && (back.position.y + (back.size.y)) > -(windowSize.y / 2.f - cameraCenter.y))
	{
		return true;
	}
	return false;
}

bool isInOfCameraE(Background& back)
{
	glm::vec2 windowSize = { static_cast<float>(Engine::GetCameraManager().GetViewSize().x),
		static_cast<float>(Engine::GetCameraManager().GetViewSize().y) };
	windowSize = (windowSize / Engine::GetCameraManager().GetZoom());
	glm::vec2 cameraCenter = Engine::GetCameraManager().GetCenter();

	if (back.position.x - (back.size.x) < (windowSize.x / 2.f + cameraCenter.x) && back.position.x + (back.size.x) > -(windowSize.x / 2.f - cameraCenter.x)
		&& back.position.y - (back.size.y) < (windowSize.y / 2.f + cameraCenter.y) && back.position.y + (back.size.y) > -(windowSize.y / 2.f - cameraCenter.y))
	{
		return true;
	}
	return false;
}

void BackgroundManager::AddNormalBackground(std::string spriteName_, glm::vec2 position_, glm::vec2 size_, float angle_, glm::vec2 speed_, glm::vec2 sizeIncrements_, float depth_, bool isScrolled_, bool isAnimated)
{
	Background temp;
	temp.spriteName = spriteName_;
	temp.position = position_;
	temp.size = size_;
	temp.speed = speed_;
	temp.angle = angle_;
	temp.sizeIncrements = sizeIncrements_;
	temp.depth = depth_;
	temp.type = BackgroundType::NORMAL;
	temp.isScrolled = isScrolled_;
	temp.isAnimation = isAnimated;

	if (isAnimated == true)
	{
		temp.sprite = new Sprite;
		temp.sprite->LoadAnimation(temp.spriteName, temp.spriteName);
		temp.sprite->UpdateModel({ temp.position.x, temp.position.y, temp.depth }, { temp.size.x, temp.size.y, 0.f }, temp.angle);
		temp.sprite->UpdateProjection();
		temp.sprite->UpdateView();
	}
	else
	{
		temp.sprite = new Sprite;
		temp.sprite->AddMeshWithTexture(temp.spriteName);
		temp.sprite->UpdateModel({ temp.position.x, temp.position.y, temp.depth }, { temp.size.x, temp.size.y, 0.f }, temp.angle);
		temp.sprite->UpdateProjection();
		temp.sprite->UpdateView();
	}

	normalBackgroundList.push_back(temp);
	//saveBackgroundList["none"].push_back(temp);
}

void BackgroundManager::AddVerticalParallexBackground(std::string spriteName_, std::string groupName, glm::vec2 position_, glm::vec2 size_, float angle_, float speedY_, float depth_, bool isAnimated)
{
	Background saveb;
	saveb.spriteName = spriteName_;
	saveb.position = position_;
	saveb.size = size_;
	saveb.speed = { 0.f, speedY_ };
	saveb.angle = angle_;
	saveb.sizeIncrements = { 0.f,0.f };
	saveb.depth = depth_;
	saveb.type = BackgroundType::VPARALLEX;
	saveb.isAnimation = isAnimated;
	//saveBackgroundList[groupName].push_back(saveb);

	glm::vec2 windowSize = { static_cast<float>(Engine::GetCameraManager().GetViewSize().x) , static_cast<float>(Engine::GetCameraManager().GetViewSize().y) };
	int amount = static_cast<int>(windowSize.y / (size_.y * 2.f)) + 1;
	if (windowSize.y < (size_.y * 2.f))
	{
		amount++;
	}
	float startPoint = position_.y;

	for (int y = 0; y < amount; y++)
	{
		Background temp;
		temp.spriteName = spriteName_;
		temp.position = { position_.x, startPoint - (size_.y * 2.f * y) };
		temp.size = size_;
		temp.speed.y = speedY_;
		temp.sizeIncrements = { 0.f,0.f };
		temp.depth = depth_;
		temp.type = BackgroundType::VPARALLEX;
		temp.isAnimation = isAnimated;

		if (isAnimated == true)
		{
			temp.sprite = new Sprite;
			temp.sprite->LoadAnimation(temp.spriteName, temp.spriteName);
			temp.sprite->UpdateModel({ temp.position.x, temp.position.y, temp.depth }, { temp.size.x, temp.size.y, 0.f }, temp.angle);
			temp.sprite->UpdateProjection();
			temp.sprite->UpdateView();
		}
		else
		{
			temp.sprite = new Sprite;
			temp.sprite->AddMeshWithTexture(temp.spriteName);
			temp.sprite->UpdateModel({ temp.position.x, temp.position.y, temp.depth }, { temp.size.x, temp.size.y, 0.f }, temp.angle);
			temp.sprite->UpdateProjection();
			temp.sprite->UpdateView();
		}

		verticalParallaxBackgroundList[groupName].push_back(temp);
	}

}

void BackgroundManager::AddHorizonParallexBackground(std::string spriteName_, std::string groupName, glm::vec2 position_, glm::vec2 size_, float angle_, float speedX_, float depth_, bool isAnimated)
{
	Background saveb;
	saveb.spriteName = spriteName_;
	saveb.position = position_;
	saveb.size = size_;
	saveb.speed = { speedX_,0.f };
	saveb.angle = angle_;
	saveb.sizeIncrements = { 0.f,0.f };
	saveb.depth = depth_;
	saveb.isAnimation = isAnimated;
	saveb.type = BackgroundType::VPARALLEX; // 수정필요
	//saveBackgroundList[groupName].push_back(saveb);

	glm::vec2 windowSize = { static_cast<float>(Engine::GetCameraManager().GetViewSize().x) , static_cast<float>(Engine::GetCameraManager().GetViewSize().y) };
	int amount = static_cast<int>(windowSize.x / (size_.x )) + 1;
	if (windowSize.x < (size_.x))
	{
		amount++;
	}
	float startPoint = position_.x;

	for (int x = 0; x < amount; x++)
	{
		Background temp;
		temp.spriteName = spriteName_;
		temp.position = { startPoint + (size_.x * x), position_.y };
		temp.size = size_;
		temp.speed.x = speedX_;
		temp.sizeIncrements = { 0.f,0.f };
		temp.depth = depth_;
		temp.type = BackgroundType::NORMAL;
		temp.isAnimation = isAnimated;

		if (isAnimated == true)
		{
			temp.sprite = new Sprite;
			temp.sprite->LoadAnimation(temp.spriteName, temp.spriteName);
			temp.sprite->UpdateModel({ temp.position.x, temp.position.y, temp.depth }, { temp.size.x, temp.size.y, 0.f }, temp.angle);
			temp.sprite->UpdateProjection();
			temp.sprite->UpdateView();
		}
		else
		{
			temp.sprite = new Sprite;
			temp.sprite->AddMeshWithTexture(temp.spriteName);
			temp.sprite->UpdateModel({ temp.position.x, temp.position.y, temp.depth }, { temp.size.x, temp.size.y, 0.f }, temp.angle);
			temp.sprite->UpdateProjection();
			temp.sprite->UpdateView();
		}

		verticalParallaxBackgroundList[groupName].push_back(temp);
	}

}

void BackgroundManager::AddSaveBackgroundList(std::string spriteName_, std::string groupName, BackgroundType type_, glm::vec2 position_, glm::vec2 size_, float angle_, glm::vec2 speed_, glm::vec2 sizeIncrements_, float depth_, bool isScrolled_, bool isAnimated)
{
	Background temp;
	temp.spriteName = spriteName_;
	temp.position = position_;
	temp.size = size_;
	temp.speed = speed_;
	temp.angle = angle_;
	temp.sizeIncrements = sizeIncrements_;
	temp.depth = depth_;
	temp.type = type_;
	temp.isScrolled = isScrolled_;
	temp.isAnimation = isAnimated;

	if (isAnimated == true)
	{
		temp.sprite = new Sprite;
		temp.sprite->LoadAnimation(temp.spriteName, temp.spriteName);
		temp.sprite->UpdateModel({ temp.position.x, temp.position.y, temp.depth }, { temp.size.x, temp.size.y, 0.f }, temp.angle);
		temp.sprite->UpdateProjection();
		temp.sprite->UpdateView();
	}
	else
	{
		temp.sprite = new Sprite;
		temp.sprite->AddMeshWithTexture(temp.spriteName);
		temp.sprite->UpdateModel({ temp.position.x, temp.position.y, temp.depth }, { temp.size.x, temp.size.y, 0.f }, temp.angle);
		temp.sprite->UpdateProjection();
		temp.sprite->UpdateView();
	}

	saveBackgroundList[groupName].push_back(temp);
}

void BackgroundManager::Update(float dt)
{
	if (isEditorMod == false)
	{
		for (auto& group : verticalParallaxBackgroundList)
		{
			for (auto& parallax : group.second)
			{
				//Engine::GetSpriteManager()->DrawSprite(parallax.spriteName, parallax.position, 0.f, parallax.size, { 1.f,1.f,1.f,1.f }, parallax.depth);

				if (isInOfCamera(parallax) == true)
				{
					parallax.sprite->UpdateModel({ parallax.position.x, parallax.position.y, parallax.depth }, { parallax.size.x, parallax.size.y, 0.f }, parallax.angle);
					parallax.sprite->UpdateProjection();
					parallax.sprite->UpdateView();

					if (parallax.isAnimation == true)
					{
						parallax.sprite->PlayAnimation(0);
					}
				}

				parallax.position.x -= parallax.speed.x;
				if (parallax.position.x <= -(Engine::GetCameraManager().GetViewSize().x / 2.f)
					+ Engine::GetCameraManager().GetCenter().x - parallax.size.x / 2.f)
				{
					parallax.position.x = Engine::GetCameraManager().GetViewSize().x / 2.f + parallax.size.x / 2.f + Engine::GetCameraManager().GetCenter().x;
				}
			}
		}

		for (auto& background : normalBackgroundList)
		{
			if (background.isScrolled == false)
			{
				background.position.x += background.speed.x * dt;
				background.position.y -= background.speed.y * dt;
			}
			if (isInOfCamera(background) == true)
			{
				if (background.isScrolled == true)
				{
					background.position.x += background.speed.x * dt;
					background.position.y -= background.speed.y * dt;
				}
				background.sprite->UpdateModel({ background.position.x, background.position.y, background.depth }, { background.size.x, background.size.y, 0.f }, background.angle);
				background.sprite->UpdateProjection();
				background.sprite->UpdateView();

				if (background.isAnimation == true)
				{
					background.sprite->PlayAnimation(0);
				}
			}
		}
	}
	else
	{
		for (auto& group : saveBackgroundList)
		{
			for (auto& background : group.second)
			{
				if (isInOfCameraE(background))
				{
					background.sprite->UpdateModel({ background.position.x, background.position.y, background.depth }, { background.size.x, background.size.y, 0.f }, background.angle);
					background.sprite->UpdateProjection();
					background.sprite->UpdateView();

					if (background.isAnimation == true)
					{
						background.sprite->PlayAnimation(0);
					}
				}
			}
		}
	}
}

void BackgroundManager::Clear()
{
	for (auto& back : normalBackgroundList)
	{
		delete back.sprite;
	}
	normalBackgroundList.clear();
	for (auto& back : verticalParallaxBackgroundList)
	{
		for (auto& back1 : back.second)
		{
			delete back1.sprite;
		}
	}
	verticalParallaxBackgroundList.clear();
	for (auto& back : saveBackgroundList)
	{
		for (auto& back1 : back.second)
		{
			if (back1.sprite != nullptr)
			{
				delete back1.sprite;
			}
		}
	}
	saveBackgroundList.clear();
}

