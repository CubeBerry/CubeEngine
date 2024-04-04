//Author: DOYEONG LEE
//Project: CubeEngine
//File: PocketBallDemo.cpp
#include "PocketBallDemo/PocketBallDemo.hpp"
#include "PocketBallDemo/Ball.hpp"
#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"

#include <iostream>
#include <cmath>

void PocketBallDemo::Init()
{
	Engine::GetVKRenderManager().LoadTexture("../Game/assets/PocketBall/White.png", "White");
	Engine::GetVKRenderManager().LoadTexture("../Game/assets/PocketBall/1.png", "1");
	Engine::GetVKRenderManager().LoadTexture("../Game/assets/PocketBall/2.png", "2");
	Engine::GetVKRenderManager().LoadTexture("../Game/assets/PocketBall/3.png", "3");
	Engine::GetVKRenderManager().LoadTexture("../Game/assets/PocketBall/4.png", "4");
	Engine::GetVKRenderManager().LoadTexture("../Game/assets/PocketBall/5.png", "5");
	Engine::GetVKRenderManager().LoadTexture("../Game/assets/PocketBall/6.png", "6");
	Engine::GetVKRenderManager().LoadTexture("../Game/assets/PocketBall/Table.png", "Table");
	Engine::GetVKRenderManager().LoadTexture("../Game/assets/PocketBall/Arrow.png", "Arrow");

	ballAmount = 7;
	pocketBallSystem = new PocketBallSystem();

	Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,-38.f,-0.1f }, glm::vec3{ 368.f*2 + 32.f, 510.f + 88.f,0.f }, "Table");
	Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
	Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture("Table");
	Engine::GetObjectManager().AddObject<Ball>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "White", BallType::WHITE, pocketBallSystem);
	pocketBallSystem->SetPlayerBall(Engine::GetObjectManager().GetLastObject());

	Engine::GetObjectManager().AddObject<Ball>(glm::vec3{ -120.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "1", BallType::OTHER, pocketBallSystem);
	Engine::GetObjectManager().AddObject<Ball>(glm::vec3{ -152.f,16.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "2", BallType::OTHER, pocketBallSystem);
	Engine::GetObjectManager().AddObject<Ball>(glm::vec3{ -152.f,-16.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "3", BallType::OTHER, pocketBallSystem);
	Engine::GetObjectManager().AddObject<Ball>(glm::vec3{ -184.f,32.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "4", BallType::OTHER, pocketBallSystem);
	Engine::GetObjectManager().AddObject<Ball>(glm::vec3{ -184.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "5", BallType::OTHER, pocketBallSystem);
	Engine::GetObjectManager().AddObject<Ball>(glm::vec3{ -184.f,-32.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "6", BallType::OTHER, pocketBallSystem);

	pocketBallSystem->Init();
	pocketBallSystem->SetBallNum(ballAmount);

	{
		glm::vec2 tempS{ 0.f,0.f };
		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,192.f,0.f }, glm::vec3{ 640.f, 32.f,0.f }, "Wall", ObjectType::WALL);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,0.f });

		tempS = { Engine::GetObjectManager().GetLastObject()->GetSize().x / 2.f,Engine::GetObjectManager().GetLastObject()->GetSize().y / 2.f };
		Engine::GetObjectManager().GetLastObject()->AddComponent<Physics2D>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, tempS.y });
		//Engine::GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, -(tempS.y - 16.f) });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x - 32.f, -tempS.y });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x + 32.f, -tempS.y });
		//Engine::GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, -(tempS.y - 16.f) });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, tempS.y });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);


		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 0.f,-192.f,0.f }, glm::vec3{ 640.f, 32.f,0.f }, "Wall", ObjectType::WALL);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,0.f });
		Engine::GetObjectManager().GetLastObject()->SetRotate(180.f);

		tempS = { Engine::GetObjectManager().GetLastObject()->GetSize().x / 2.f,Engine::GetObjectManager().GetLastObject()->GetSize().y / 2.f };
		Engine::GetObjectManager().GetLastObject()->AddComponent<Physics2D>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, tempS.y });
		//Engine::GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, -(tempS.y - 16.f) });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x - 32.f, -tempS.y });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x + 32.f, -tempS.y });
		//Engine::GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, -(tempS.y - 16.f) });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, tempS.y });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);


		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -336.f,0.f,0.f }, glm::vec3{ 352.f, 32.f,0.f }, "Wall", ObjectType::WALL);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,0.f });

		tempS = { Engine::GetObjectManager().GetLastObject()->GetSize().x / 2.f,Engine::GetObjectManager().GetLastObject()->GetSize().y / 2.f };
		Engine::GetObjectManager().GetLastObject()->AddComponent<Physics2D>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, tempS.y });
		//Engine::GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, -(tempS.y - 16.f) });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x - 32.f, -tempS.y });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x + 32.f, -tempS.y });
		//Engine::GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, -(tempS.y - 16.f) });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, tempS.y });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);
		Engine::GetObjectManager().GetLastObject()->SetRotate(90.f);

		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 336.f,0.f,0.f }, glm::vec3{ 352.f, 32.f,0.f }, "Wall", ObjectType::WALL);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,0.f });

		tempS = { Engine::GetObjectManager().GetLastObject()->GetSize().x / 2.f,Engine::GetObjectManager().GetLastObject()->GetSize().y / 2.f };
		Engine::GetObjectManager().GetLastObject()->AddComponent<Physics2D>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, tempS.y });
		//Engine::GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x, -(tempS.y - 16.f) });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ tempS.x - 32.f, -tempS.y });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x + 32.f, -tempS.y });
		//Engine::GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, -(tempS.y - 16.f) });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -tempS.x, tempS.y });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);
		Engine::GetObjectManager().GetLastObject()->SetRotate(270.f);

		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 336.f,192.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Goal", ObjectType::GOAL);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddQuad({ 0.f,0.f,0.f,0.5f });

		Engine::GetObjectManager().GetLastObject()->AddComponent<Physics2D>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 2.f, 2.f });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);

		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -336.f,192.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Goal", ObjectType::GOAL);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddQuad({ 0.f,0.f,0.f,0.5f });

		Engine::GetObjectManager().GetLastObject()->AddComponent<Physics2D>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 2.f, 2.f });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);

		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ -336.f,-192.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Goal", ObjectType::GOAL);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddQuad({ 0.f,0.f,0.f,0.5f });

		Engine::GetObjectManager().GetLastObject()->AddComponent<Physics2D>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 2.f, 2.f });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);

		Engine::GetObjectManager().AddObject<Object>(glm::vec3{ 336.f,-192.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "Goal", ObjectType::GOAL);
		Engine::GetObjectManager().GetLastObject()->AddComponent<Sprite>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Sprite>()->AddQuad({ 0.f,0.f,0.f,0.5f });

		Engine::GetObjectManager().GetLastObject()->AddComponent<Physics2D>();
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 2.f, 2.f });
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::BLOCK);
		Engine::GetObjectManager().GetLastObject()->GetComponent<Physics2D>()->SetMass(4.f);
	}
}

void PocketBallDemo::Update(float dt)
{
	if (Engine::GetInputManager().IsKeyPressedOnce(KEYBOARDKEYS::NUMBER_1))
	{
		Engine::GetGameStateManager().ChangeLevel(GameLevel::POCKETBALL);
	}
	else if (Engine::GetInputManager().IsKeyPressedOnce(KEYBOARDKEYS::NUMBER_2))
	{
		Engine::GetGameStateManager().ChangeLevel(GameLevel::PLATFORMDEMO);
	}
	pocketBallSystem->Update(dt);
}

#ifdef _DEBUG
void PocketBallDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
	Engine::GetGameStateManager().StateChanger();
	Engine::GetSoundManager().MusicPlayerForImGui();
}
#endif

void PocketBallDemo::Restart()
{
	End();
	Init();
}

void PocketBallDemo::End()
{
	if(pocketBallSystem != nullptr)
	{
		pocketBallSystem->End();
	}

	delete pocketBallSystem;
	pocketBallSystem = nullptr;

	Engine::GetCameraManager().Reset();
	Engine::GetParticleManager().Clear();
	Engine::GetObjectManager().DestroyAllObjects();
}

