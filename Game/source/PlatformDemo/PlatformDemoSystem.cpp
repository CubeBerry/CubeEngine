//Author: DOYEONG LEE
//Project: CubeEngine
//File: PlatformDemoSystem.cpp
#include "PlatformDemo/PlatformDemoSystem.hpp"
#include "PlatformDemo/PPlayer.hpp"
#include "Engine.hpp"

#include <iostream>
#include <fstream>

#ifdef _DEBUG
#include "imgui.h"
#endif


void PDemoMapEditorDemo::LoadLevelData(const std::filesystem::path& filePath)
{
	std::ifstream inStream;
	inStream.open(filePath);

	if (inStream.is_open() == false)
	{
		std::cout << "Fail to Access file";
	}

	while (!inStream.eof())
	{
		float posX;
		float posY;
		float sizeX;
		float sizeY;

		std::string objectName;
		std::string spriteName;
		std::string objectType = "";

		inStream >> objectName;
		inStream >> posX;
		inStream >> posY;
		inStream >> sizeX;
		inStream >> sizeY;
		inStream >> objectType;
		//inStream >> spriteName;


		if (objectType == "PLAYER")
		{
			if (isEditorMod == false)
			{
				Engine::Instance().GetObjectManager()->AddObject<PPlayer>(glm::vec3{ posX, posY, 0.f }, glm::vec3{ sizeX, sizeY, 0.f }, "Player", ObjectType::PLAYER);
			}
			else
			{
				PPlayer* temp = new PPlayer(glm::vec3{ posX, posY, 0.f }, glm::vec3{ sizeX, sizeY, 0.f }, "Player", ObjectType::PLAYER);
				objects.push_back(std::move(temp));
			}
		}
		if (objectType == "WALL")
		{
			if (isEditorMod == false)
			{
				Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ posX, posY, 0.f }, glm::vec3{ sizeX, sizeY, 0.f }, "Wall", ObjectType::WALL);
				Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
				Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 0.5f,0.5f,0.5f,1.f });

				Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
				Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().x / 2.f,  Engine::Instance().GetObjectManager()->GetLastObject()->GetSize().y / 2.f });
				Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
				Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(1.f);
			}
			else
			{
				Object* temp = new Object(glm::vec3{ posX, posY,0.f }, glm::vec3{ sizeX, sizeY, 0.f }, "Wall", ObjectType::WALL);
				temp->AddComponent<Sprite>();
				temp->GetComponent<Sprite>()->AddQuad({ 0.f,1.f,0.f,0.25f });

				temp->AddComponent<Physics2D>();
				temp->GetComponent<Physics2D>()->AddCollidePolygonAABB({ temp->GetSize().x / 2.f,  temp->GetSize().y / 2.f });
				temp->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
				temp->GetComponent<Physics2D>()->SetMass(1.f);
				walls.push_back(std::move(temp));
			}
		}
	}
	inStream.close();
}

void PDemoMapEditorDemo::SaveLevelData(const std::filesystem::path& outFilePath)
{
	std::ofstream saveLoad(outFilePath);
	for (auto& target : walls)
	{
		saveLoad << target->GetName() << ' ';
		saveLoad << target->GetPosition().x << ' ';
		saveLoad << target->GetPosition().y << ' ';
		saveLoad << target->GetSize().x << ' ';
		saveLoad << target->GetSize().y << ' ';
		switch (target->GetObjectType())
		{
		case ObjectType::PLAYER:
			saveLoad << "PLAYER" << ' ';
			break;
		case ObjectType::WALL:
			saveLoad << "WALL" << ' ';
			break;
		}

		saveLoad << '\n';
	}
	saveLoad.close();
}

#ifdef _DEBUG
void PDemoMapEditorDemo::Init()
{
	//if (isEditorMod == true)
	//{
		target = new Target();
		target->rect = new Sprite();
		target->rect->AddQuad({ 0.f,1.f,0.f,0.0f });
	//}
}

void PDemoMapEditorDemo::Update(float dt)
{
	if (isEditorMod == true)
	{
		for (auto& o : walls)
		{
			o->GetComponent<Physics2D>()->Update(dt);
			o->GetComponent<Sprite>()->Update(dt);
		}

		ImGui::Begin("MapEditor");
		//Button
		if (ImGui::Button("SaveFile"))
		{
			SaveLevelData("../Game/assets/PlatformDemo/Stage.txt");
		}


		if (ImGui::Button("ObjectCreator"))
		{
			mode = EditorMode::OBJCREATOR;
		}
		ImGui::SameLine();
		if (ImGui::Button("WallCreator"))
		{
			mode = EditorMode::WALLCREATOR;
		}
		ImGui::End();

		switch (mode)
		{
		case EditorMode::OBJCREATOR:
			target->rect->SetColor({ 1.f,1.f,1.f,1.f });
			ObjectCreator();
			break;
		case EditorMode::WALLCREATOR:
			target->rect->SetColor({ 0.f,1.f,0.f,1.f });
			WallCreator();
			break;
		}
	}
}
void PDemoMapEditorDemo::End()
{
	delete target->rect;
	delete target;
	for (auto* target : objects)
	{
		delete target;
	}
	for (auto* target : walls)
	{
		delete target;
	}
	objects.clear();
	walls.clear();
}
#endif

void PDemoMapEditorDemo::ObjectCreator()
{
	float  targetP[2] = { target->pos.x, target->pos.y };
	float  targetScale[2] = { target->size.x, target->size.y };

	ImGui::Begin("ObjectCreator");
	//SpriteList(newObject);
	//DrawTypeList(newObject);
	//ObjectTypeList(newObject);

	static char newName[256] = "none";
	ImGui::InputText("default", newName, 256);
	target->name = newName;

	ImGui::InputFloat2("Position", targetP);
	glm::vec2 mPos = Engine::Instance().GetInputManager()->GetMousePosition();
	glm::vec2 newPosition = { mPos.x - std::fmod(mPos.x, gridSize.x),  mPos.y - std::fmod(mPos.y, gridSize.y) };
	target->pos = newPosition;

	ImGui::InputFloat2("Scale", targetScale);
	glm::vec2  newSize = { targetScale[0], targetScale[1] };
	target->size = newSize;
}

void PDemoMapEditorDemo::WallCreator()
{
	//ImGuiIO& io = ImGui::GetIO();
	//ImVec2 cursor_pos = io.MousePos;
	//bool cursor_over_imgui_window = ImGui::IsMouseHoveringAnyWindow();
	//if()
	{
		float distance = glm::distance(target->startPos, target->endPos);
		glm::vec2 midPoint = { (target->startPos.x + target->endPos.x) / 2.f, (target->startPos.y + target->endPos.y) / 2.f };
		if (isWallSetting == false)
		{
			glm::vec2 mPos = Engine::Instance().GetInputManager()->GetMousePosition();
			target->startPos = { mPos.x - std::fmod(mPos.x, gridSize.x),  mPos.y - std::fmod(mPos.y, gridSize.y) };
			target->rect->UpdateModel({ target->startPos.x, -target->startPos.y, 0.f }, { 4.f,4.f,0.f }, 0.f);
		}
		else
		{
			glm::vec2 mPos = Engine::Instance().GetInputManager()->GetMousePosition();
			target->endPos = { mPos.x - std::fmod(mPos.x, gridSize.x),  mPos.y - std::fmod(mPos.y, gridSize.y) };
			target->rect->UpdateModel({ midPoint.x, -midPoint.y, 0.f }, { abs(target->endPos.x - target->startPos.x) , abs(target->endPos.y - target->startPos.y),0.f }, 0.f);
		}
		target->rect->UpdateProjection();
		target->rect->UpdateView();

		if (Engine::Instance().GetInputManager()->IsMouseButtonPressedOnce(MOUSEBUTTON::LEFT))
		{
			if (isWallSetting == true)
			{
				Object* temp = new Object(glm::vec3{ midPoint.x, -midPoint.y,0.f }, glm::vec3{ abs(target->endPos.x - target->startPos.x) , abs(target->endPos.y - target->startPos.y),0.f }, "Wall", ObjectType::WALL);
				temp->AddComponent<Sprite>();
				temp->GetComponent<Sprite>()->AddQuad({ 0.f,1.f,0.f,0.25f });

				temp->AddComponent<Physics2D>();
				temp->GetComponent<Physics2D>()->AddCollidePolygonAABB({ temp->GetSize().x / 2.f,  temp->GetSize().y / 2.f });
				temp->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
				temp->GetComponent<Physics2D>()->SetMass(1.f);
				walls.push_back(std::move(temp));
				isWallSetting = false;
			}
			else
			{
				isWallSetting = true;
			}
		}
		else if (Engine::Instance().GetInputManager()->IsMouseButtonPressedOnce(MOUSEBUTTON::RIGHT))
		{
			if (isWallSetting == true)
			{
				isWallSetting = false;
			}
		}
	}
}

void PlatformDemoSystem::Init()
{
	mapEditor = new PDemoMapEditorDemo();
	healthBar = new Sprite();
	healthBar->AddQuad({ 0.f,1.f,0.f,1.f });
#ifdef _DEBUG
	mapEditor->Init();
#endif
}

void PlatformDemoSystem::Update(float dt)
{
	glm::vec2 viewSize = Engine::GetCameraManager()->GetViewSize();
	glm::vec2 center = Engine::GetCameraManager()->GetCenter();
	healthBar->UpdateModel({ (-viewSize.x / 2.f + 320.f) + center.x , (viewSize.y / 2.f - 128.f) + center.y, 0.f }, { 320.f * (1.f / maxHp * hp), 64.f, 0.f }, 0.f);
	healthBar->UpdateProjection();
	healthBar->UpdateView();

#ifdef _DEBUG
	mapEditor->Update(dt);
#endif
}

void PlatformDemoSystem::End()
{
	delete healthBar;
	healthBar = nullptr;
#ifdef _DEBUG
	mapEditor->End();
	delete mapEditor;
#endif
}

#ifdef _DEBUG
void PlatformDemoSystem::UpdateMapEditor(float dt)
{
	mapEditor->Update(dt);
}
#endif
