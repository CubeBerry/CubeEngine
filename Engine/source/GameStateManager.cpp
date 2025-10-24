//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: GameStateManager.cpp
#include "GameStateManager.hpp"
#include "Engine.hpp"

#include "BasicComponents/Physics3D.hpp"
#include "imgui.h"

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
	Engine::GetObjectManager().ProcessFunctionQueue();
	state = State::UPDATE;
#ifdef _DEBUG
	std::cout << "Load Complete" << '\n';
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
		Engine::GetObjectManager().ProcessFunctionQueue();
		Engine::Instance().GetTimer().Init(Engine::Instance().GetTimer().GetFrameRate());
#ifdef _DEBUG
		std::cout << "Load Complete" << '\n';
#endif
		state = State::UPDATE;
#ifdef _DEBUG
		std::cout << "Update" << '\n';
#endif
		break;
	case State::UPDATE:
		UpdateGameLogic(dt);
		UpdateDraw(dt);
		Engine::GetObjectManager().ProcessFunctionQueue();
		Engine::GetObjectManager().DeleteObjectsFromList();
		Engine::GetRenderManager()->ProcessFunctionQueue();
		//Mouse Input X if order is opposite
		break;
	case State::PAUSE:
		UpdateGameLogic(0.f);
		UpdateDraw(0.f);
		Engine::GetObjectManager().ProcessFunctionQueue();
		Engine::GetObjectManager().DeleteObjectsFromList();
		break;
	case State::CHANGE:
		// @TODO temporary function to clear all textures
		Engine::GetRenderManager()->ClearTextures();
		levelList.at(static_cast<int>(currentLevel))->End();
		currentLevel = levelSelected;
#ifdef _DEBUG
		std::cout << "Level Change" << '\n';
#endif
		state = State::LOAD;
		break;
	case State::RESTART:
		LevelEnd();
		state = State::LOAD;
#ifdef _DEBUG
		std::cout << "Level Restart" << '\n';
#endif
		break;
	case State::UNLOAD:
		LevelEnd();
		state = State::SHUTDOWN;
#ifdef _DEBUG
		std::cout << "ShutDown" << '\n';
#endif
		break;
	case State::SHUTDOWN:
		break;
	}
}

void GameStateManager::Draw()
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->BeginRender({ 0.0f, 0.0f, 0.0f });
	renderManager->EndRender();

	//VK Draw
	//VKRenderManager* renderManager = &Engine::Instance().GetVKRenderManager();
	//renderManager->BeginRender();
	//renderManager->EndRender();

	//GL Draw
	//GLRenderManager* renderManager = &Engine::Instance().GetGLRenderManager();
	//renderManager->BeginRender();
	//renderManager->EndRender();
}

void GameStateManager::DrawWithImGui(float dt)
{
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	if (renderManager->BeginRender({ 0.0f, 0.0f, 0.0f }))
	{
		if (showFPSHistory) Engine::Instance().GetTimer().ShowFpsGraph();
		StateChanger();
		levelList.at(static_cast<int>(currentLevel))->ImGuiDraw(dt);
		if (renderManager->GetRenderType() == RenderType::ThreeDimension) renderManager->RenderingControllerForImGui();
		renderManager->EndRender();
	}

	//VK Draw
	//VKRenderManager* renderManager = &Engine::Instance().GetVKRenderManager();
	//renderManager->BeginRender();
	//levelList.at(static_cast<int>(currentLevel))->ImGuiDraw(dt);
	//renderManager->EndRender();

	//GL Draw
	//GLRenderManager* renderManager = &Engine::Instance().GetGLRenderManager();
	//renderManager->BeginRender();
	//levelList.at(static_cast<int>(currentLevel))->ImGuiDraw(dt);
	//renderManager->EndRender();
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

void GameStateManager::UpdateGameLogic(float dt)
{
	if (levelSelected != currentLevel)
	{
		state = State::CHANGE;
	}
	else
	{
		levelList.at(static_cast<int>(currentLevel))->Update(dt);
		Engine::GetObjectManager().Update(dt);
		Engine::GetParticleManager().Update(dt);
		Engine::GetCameraManager().Update();
		CollideObjects();
		Engine::GetSpriteManager().Update(dt);
	}
}
void GameStateManager::UpdateDraw(float dt)
{
	if (!(SDL_GetWindowFlags(Engine::GetWindow().GetWindow()) & SDL_WINDOW_MINIMIZED))
	{
		DrawWithImGui(dt);
		//Draw();
	}
}

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
					showDescription = false;
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("FPS"))
		{
			if (ImGui::MenuItem("30"))
			{
				Engine::Instance().SetFPS(FrameRate::FPS_30);
			}
			if (ImGui::MenuItem("60"))
			{
				Engine::Instance().SetFPS(FrameRate::FPS_60);
			}
			if (ImGui::MenuItem("120"))
			{
				Engine::Instance().SetFPS(FrameRate::FPS_120);
			}
			if (ImGui::MenuItem("144"))
			{
				Engine::Instance().SetFPS(FrameRate::FPS_144);
			}
			if (ImGui::MenuItem("240"))
			{
				Engine::Instance().SetFPS(FrameRate::FPS_240);
			}
			if (ImGui::MenuItem("UNLIMIT"))
			{
				Engine::Instance().SetFPS(FrameRate::UNLIMIT);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Toggle FPS History"))
			{
				showFPSHistory = !showFPSHistory;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("How To Control"))
			{
				showDescription = true;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (showDescription)
	{
		ImGui::OpenPopup("DescriptionPopup");
		showDescription = false;
	}

	if (ImGui::BeginPopup("DescriptionPopup"))
	{
		switch (currentLevel)
		{
		case GameLevel::PROCEDURALMESHES:
			ImGui::Text("PROCEDURALMESHES DEMO                              ");
			ImGui::Separator();

			ImGui::TextWrapped("Move : Arrow Keys\nMove Up/Down : Space Bar/Left Shift\nMove Camera view: Drag with Mouse Right Click\nYou can adjust the mesh's components via the control panel.");
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			break;
		case GameLevel::VERTICES:
			ImGui::Text("This is a test level");
			ImGui::Separator();
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			break;
		case GameLevel::PHYSICSDEMO:
			ImGui::Text("PHYSICS DEMO          ");
			ImGui::Separator();

			ImGui::TextWrapped("Move CUBE: Arrow Keys");
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			break;
		case GameLevel::POCKETBALL:
			ImGui::Text("POCKETBALL DEMO          ");
			ImGui::Separator();

			ImGui::TextWrapped("Move Cursor: Arrow Keys\nShot : SpaceBar\n");
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			break;
		case GameLevel::PLATFORMDEMO:
			ImGui::Text("PLATFORM DEMO          ");
			ImGui::Separator();

			ImGui::TextWrapped("Move : Arrow Keys\nAttack : Z\nJump : X");
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			break;
		case GameLevel::BEATEMUPDEMO:
			ImGui::Text("BEATEMUP DEMO          ");
			ImGui::Separator();

			ImGui::TextWrapped("Move : Arrow Keys\nAttack : Z\nJump : X");
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			break;
		}
		ImGui::EndPopup();
	}
}

const char* GameStateManager::GameLevelTypeEnumToChar(GameLevel type)
{
	switch (type)
	{
	case GameLevel::PROCEDURALMESHES:
		return "PROCEDURALMESHES";
		break;
	case GameLevel::VERTICES:
		return "VERTICES";
		break;
	case GameLevel::PHYSICSDEMO:
		return "PHYSICSDEMO";
		break;
	case GameLevel::POCKETBALL:
		return "POCKETBALL";
		break;
	case GameLevel::PLATFORMDEMO:
		return "PLATFORMDEMO";
		break;
	case GameLevel::BEATEMUPDEMO:
		return "BEATEMUPDEMO";
		break;
	case GameLevel::PBR:
		return "PBR";
		break;
	}
	return "NONE";
}

void GameStateManager::CollideObjects()
{
	for (auto& target : Engine::GetObjectManager().GetObjectMap())
	{
		for (auto& object : Engine::GetObjectManager().GetObjectMap())
		{
			if (target.second != nullptr && object.second != nullptr && target.second != object.second)
			{
				if (target.second->HasComponent<Physics2D>() == true && object.second->HasComponent<Physics2D>() == true)
				{
					target.second->CollideObject(object.second.get());
				}
				else if (target.second->HasComponent<Physics3D>() == true && object.second->HasComponent<Physics3D>() == true)
				{
					target.second->CollideObject(object.second.get());
				}
			}
		}
	}
}
