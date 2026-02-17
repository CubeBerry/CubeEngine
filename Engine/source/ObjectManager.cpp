//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: ObjectManager.cpp
#include "ObjectManager.hpp"

#include "Engine.hpp"
#include "GLRenderManager.hpp"
#include "VKRenderManager.hpp"
#include "DXRenderManager.hpp"

#include "BasicComponents/Physics3D.hpp"
#include "BasicComponents/Light.hpp"
#include "BasicComponents/SkeletalAnimator.hpp"
#include "BasicComponents/SkeletalAnimationStateMachine.hpp"
#include "SkeletalAnimation/SkeletalAnimation.hpp"

#include "imgui.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Helper function to world-to-screen transform for ImGui drawing
glm::vec2 WorldToScreen(glm::vec3 worldPos, glm::mat4 view, glm::mat4 proj)
{
	// Transform world space to clip space
	glm::vec4 clipSpace = proj * view * glm::vec4(worldPos, 1.0f);

	// Discard points behind the camera
	if (clipSpace.w <= 0.0f) return glm::vec2(-1, -1);

	// Perspective divide to get Normalized Device Coordinates (NDC)
	glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;

	// Map NDC to screen coordinates using ImGui viewport data
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	glm::vec2 windowPos = { viewport->Pos.x, viewport->Pos.y };
	glm::vec2 windowSize = { viewport->Size.x, viewport->Size.y };

	// Convert NDC to screen space and flip Y-axis for ImGui coordinate system
	float screenX = (ndc.x + 1.0f) * 0.5f * windowSize.x + windowPos.x;
	float screenY = (1.0f - ndc.y) * 0.5f * windowSize.y + windowPos.y;

	return glm::vec2(screenX, screenY);
}

ObjectManager::ObjectManager()
{
	//Engine::GetLogger().LogDebug(LogCategory::Engine, "Object Manager Initialized");
}

ObjectManager::~ObjectManager()
{
	Engine::GetLogger().LogDebug(LogCategory::Engine, "Object Manager Deleted");
}

void ObjectManager::Update(float dt)
{
	std::for_each(objectMap.begin(), objectMap.end(), [&](auto& obj) { obj.second->Update(dt); });
	//DeleteObjectsFromList();
}

void ObjectManager::DeleteObjectsFromList()
{
	std::for_each(objectsToBeDeleted.begin(), objectsToBeDeleted.end(), [&](int id) { objectMap.at(id).reset(); objectMap.erase(id); });
	objectsToBeDeleted.clear();
}

void ObjectManager::End()
{
	objName = "Object";
	DestroyAllObjects();
	currentIndex = 0;
	objectListForImguiIndex = 0;
	functionQueue.clear();
}

void ObjectManager::Draw(float dt)
{
	std::ranges::for_each(objectMap.begin(), objectMap.end(), [&](auto& obj) { obj.second->Draw(dt); });
}

void ObjectManager::Destroy(int id)
{
	//objectsToBeDeleted.push_back(id);
	//objectMap.at(id).get()->DestroyAllComponents();

	objectsToBeDeleted.push_back(id);
	//std::for_each(objectsToBeDeleted.begin(), objectsToBeDeleted.end(), [&](int id) { objectMap.at(id).reset(); objectMap.erase(id); });
	//objectsToBeDeleted.clear();
}

void ObjectManager::DestroyAllObjects()
{
	lastObjectID = 0;
	for (auto& obj : objectMap)
	{
		obj.second.reset();
	}
	objectsToBeDeleted.clear();
	objectMap.clear();
}

int ObjectManager::GetLastObjectID()
{
	if (!objectMap.empty())
	{
		return (GetLastObject()->GetId());
	}
	return 0;
}

Object* ObjectManager::FindObjectWithName(std::string name)
{
	for (auto& obj : objectMap)
	{
		if (obj.second.get()->GetName() == name)
		{
			return obj.second.get();
		}
	}
	return nullptr;
}

void ObjectManager::ObjectControllerForImGui()
{
	ImGui::Begin("ObjectController");

	static char newName[256] = "Object";
	ImGui::InputText("Object Name", newName, 128);
	objName = newName;

	if (ImGui::Button("Add New Object"))
	{
		AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1.f,1.f,1.f }, objName, ObjectType::NONE);
	}

	ImGui::SameLine();
	if (!objectMap.empty())
	{
		if (ImGui::Button("Delete"))
		{
			Destroy(currentIndex);
		}

		if (currentIndex > GetLastObjectID())
		{
			objectListForImguiIndex--;
			currentIndex = GetLastObjectID();
		}

		ImGui::Separator();
		ImGui::Spacing();

		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, 3 * ImGui::GetTextLineHeightWithSpacing()));
		ImGui::BeginChild("Scrolling");
		int index = 0;
		for (auto& object : GetObjectMap())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, (objectListForImguiIndex == index) ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Text));
			std::string indexName = std::to_string(index) + "." + object.second.get()->GetName();
			if ((ImGui::Selectable(indexName.c_str(), currentIndex)) || objectListForImguiIndex == index)
			{
				objectListForImguiIndex = index;
				currentIndex = object.second.get()->GetId();
			}
			ImGui::PopStyleColor();
			index++;
		}

		ImGui::EndChild();
		ImGui::Separator();
		ImGui::Spacing();

		Object* obj = FindObjectWithId(currentIndex);

		glm::vec3 originalPosition = obj->GetPosition();
		glm::vec3 position = originalPosition;
		glm::vec3 size = obj->GetSize();
		glm::vec3 rotation = obj->GetRotate3D();

		auto* isCurrentLight = obj->GetComponent<Light>();
		if (!isCurrentLight || (isCurrentLight && isCurrentLight->GetLightType() != LightType::DIRECTIONAL))
		{
			if (ImGui::CollapsingHeader("Object Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat3("Object Position", &position.x, 0.01f);
				ImGui::DragFloat3("Object Size", &size.x, 0.5f);
				ImGui::DragFloat3("Object Rotation", &rotation.x, 0.5f);

				obj->SetPosition(position);

				if (originalPosition != position)
				{
					if (obj->HasComponent<Physics3D>())
					{
						// If the component exists, call the Teleport function.
						obj->GetComponent<Physics3D>()->Teleport(position);
					}
					else
					{
						// If the component does not exist, call the original SetPosition function.
						obj->SetPosition(position);
					}
				}
				obj->SetXSize(size.x);
				obj->SetYSize(size.y);
				obj->SetZSize(size.z);
				obj->SetRotate(rotation);
			}
		}

		for (auto* comp : obj->GetComponentList())
		{
			ComponentTypes type = comp->GetType();
			switch (type)
			{
			case ComponentTypes::SPRITE:
				SpriteControllerForImGui(reinterpret_cast<DynamicSprite*>(comp));
				break;
			case ComponentTypes::PHYSICS3D:
				Physics3DControllerForImGui(reinterpret_cast<Physics3D*>(comp));
				break;
			case ComponentTypes::LIGHT:
				LightControllerForImGui(reinterpret_cast<Light*>(comp));
				break;
			case ComponentTypes::SKETANIMATOR:
				SkeletalAnimatorControllerForImGui(reinterpret_cast<SkeletalAnimator*>(comp));
				break;
			case ComponentTypes::SKETANIMASTATE:
				AnimationStateMachineControllerForImGui(reinterpret_cast<SkeletalAnimationStateMachine*>(comp));
				break;
			}
		}

		ImGui::Separator();
		ImGui::Spacing();
		if (ImGui::Button("Add Component"))
		{
			isShowPopup = true;
			ImGui::OpenPopup("Select Component");
		}

		obj = nullptr;

		if (ImGui::IsPopupOpen("Select Component"))
		{
			AddComponentPopUpForImGui();
		}
		if (ImGui::IsPopupOpen("Select Model"))
		{
			SelectObjModelPopUpForImGui();
		}
	}

	ImGui::End();
	//SelectObjectWithMouse();

	if (isShowPopup == true && Engine::GetGameStateManager().GetGameState() == State::UPDATE)
	{
		Engine::GetGameStateManager().SetGameState(State::PAUSE);
	}
	else if (isShowPopup == false && Engine::GetGameStateManager().GetGameState() == State::PAUSE)
	{
		Engine::GetGameStateManager().SetGameState(State::UPDATE);
	}
}

void ObjectManager::ProcessFunctionQueue()
{
	for (auto& task : functionQueue)
	{
		task();
	}
	functionQueue.clear();
}

void ObjectManager::Physics3DControllerForImGui(Physics3D* phy)
{
	Physics3D* physics = phy;
	if (ImGui::CollapsingHeader("Physics3D", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool isGravityOn = physics->GetIsGravityOn();
		ImGui::Checkbox("Use Gravity", &isGravityOn);
		physics->SetGravity(physics->GetGravity(), isGravityOn);

		glm::vec3 velocity = physics->GetVelocity();
		ImGui::DragFloat3("Velocity", &velocity.x, 0.01f);
		physics->SetVelocity(velocity);

		glm::vec3 minVelocity = physics->GetMinVelocity();
		ImGui::DragFloat3("Min Velocity", &minVelocity.x, 0.01f);
		physics->SetMinVelocity(minVelocity);

		glm::vec3 maxVelocity = physics->GetMaxVelocity();
		ImGui::DragFloat3("Max Velocity", &maxVelocity.x, 0.01f);
		physics->SetMaxVelocity(maxVelocity);

		float gravity = physics->GetGravity();
		ImGui::DragFloat("Gravity", &gravity, 0.01f);
		physics->SetGravity(gravity, isGravityOn);

		float friction = physics->GetFriction();
		ImGui::DragFloat("Friction", &friction, 0.01f);
		physics->SetFriction(friction);

		float mass = physics->GetMass();
		ImGui::DragFloat("Mass", &mass, 0.01f);
		physics->SetMass(mass);

		ImGui::SeparatorText("Collision");
		int mode_int = static_cast<int>(phy->GetCollisionDetectionMode());

		ImGui::RadioButton("Discrete", &mode_int, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Continuous", &mode_int, 1);

		if (mode_int != static_cast<int>(phy->GetCollisionDetectionMode()))
		{
			phy->SetCollisionDetectionMode(static_cast<CollisionDetectionMode>(mode_int));
		}

		if (ImGui::RadioButton("RIGID", physics->GetBodyType() == BodyType3D::RIGID))
		{
			physics->SetBodyType(BodyType3D::RIGID);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("BLOCK", physics->GetBodyType() == BodyType3D::BLOCK))
		{
			physics->SetBodyType(BodyType3D::BLOCK);
		}

		//bool isGhostCollisionOn = physics->GetIsGhostCollision();
		//ImGui::Checkbox("Use GhostCollision", &isGhostCollisionOn);
		//physics->SetIsGhostCollision(isGhostCollisionOn);
	}
	physics = nullptr;
}

void ObjectManager::SpriteControllerForImGui(DynamicSprite* sprite)
{
	RenderManager* renderManager = Engine::GetRenderManager();
	DynamicSprite* spriteComp = sprite;
	bool isSelectedObjModel = false;
	Object* currentObj = FindObjectWithId(currentIndex);

	if (renderManager->GetRenderType() == RenderType::ThreeDimension)
	{
		stacks = sprite->GetStacks();
		slices = sprite->GetSlices();
		metallic = sprite->GetSubMeshes()[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.metallic;
		roughness = sprite->GetSubMeshes()[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.roughness;

		if (ImGui::CollapsingHeader("3DMesh", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Spacing();
			glm::vec4 color = spriteComp->GetColor();
			if (ImGui::ColorEdit4("Color", &color.r))
			{
				spriteComp->SetColor(color);
			}

			ImGui::Spacing();
			if (ImGui::SliderInt("Stacks", &stacks, 2, 30))
			{
				MeshType meshType = sprite->GetMeshType();
				std::filesystem::path modelFilePath = sprite->GetModelFilePath();

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(meshType, modelFilePath, stacks, slices, color, metallic, roughness); });
			}
			if (ImGui::SliderInt("Slices", &slices, 2, 30))
			{
				MeshType meshType = sprite->GetMeshType();
				std::filesystem::path modelFilePath = sprite->GetModelFilePath();

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(meshType, modelFilePath, stacks, slices, color, metallic, roughness); });
			}
			if (ImGui::SliderFloat("Metallic", &metallic, 0.f, 1.f))
			{
				sprite->GetSubMeshes()[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.metallic = metallic;

			}
			if (ImGui::SliderFloat("Roughness", &roughness, 0.f, 1.f))
			{
				sprite->GetSubMeshes()[0]->GetData<BufferWrapper::DynamicSprite3DMesh>()->material.roughness = roughness;
			}
			ImGui::Spacing();

			bool isTextureExist = false;
			switch (renderManager->GetGraphicsMode())
			{
			case GraphicsMode::GL:
			{
				isTextureExist = !dynamic_cast<GLRenderManager*>(renderManager)->GetTextures().empty();
				break;
			}
			case GraphicsMode::VK:
			{
				isTextureExist = !dynamic_cast<VKRenderManager*>(renderManager)->GetTextures().empty();
				break;
			}
			case GraphicsMode::DX:
			{
				//isTextureExist = !dynamic_cast<DXRenderManager*>(renderManager)->GetTextures().empty();
				break;
			}
			}

			if (isTextureExist)
			{
				bool isTextureOn = sprite->GetIsTex();
				if (ImGui::Checkbox("Apply Texture", &isTextureOn))
				{
				}

				if (isTextureOn)
				{
					switch (renderManager->GetGraphicsMode())
					{
					case GraphicsMode::GL:
					{
						GLRenderManager* renderManagerGL = dynamic_cast<GLRenderManager*>(renderManager);
						if (ImGui::BeginCombo("TextureList", sprite->GetTextureName().c_str()))
						{
							for (auto& tex : renderManagerGL->GetTextures())
							{
								int index = 0;
								const bool is_selected = tex->GetName().c_str() == sprite->GetTextureName().c_str();

								//ImGui::Image((void*)(intptr_t)tex->GetTextureHandle(), ImVec2(16, 16), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
								//ImGui::SameLine();

								if (ImGui::Selectable(tex->GetName().c_str()))
								{
									sprite->ChangeTexture(tex->GetName());
								}
								if (is_selected)
								{
									ImGui::SetItemDefaultFocus();
								}
								index++;
							}
							ImGui::EndCombo();
						}
						break;
					}
					case GraphicsMode::VK:
					{
						VKRenderManager* renderManagerVK = dynamic_cast<VKRenderManager*>(renderManager);

						//VKTexture* objTex = renderManagerVK->GetTexture(sprite->GetTextureName().c_str());
						//if(objTex != nullptr)
						//{
						//	VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(*objTex->GetSampler(), *objTex->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
						//	ImGui::Image((ImTextureID)descriptorSet, ImVec2(64, 64), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
						//	ImGui::SameLine();
						//}

						if (ImGui::BeginCombo("TextureList", sprite->GetTextureName().c_str()))
						{
							for (auto& tex : renderManagerVK->GetTextures())
							{
								int index = 0;
								const bool is_selected = tex->GetName().c_str() == sprite->GetTextureName().c_str();

								//VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(*tex->GetSampler(), *tex->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
								//ImGui::Image((ImTextureID)descriptorSet, ImVec2(16, 16), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
								//ImGui::SameLine();

								if (ImGui::Selectable(tex->GetName().c_str()))
								{
									sprite->ChangeTexture(tex->GetName());
								}
								if (is_selected)
								{
									ImGui::SetItemDefaultFocus();
								}
								index++;
							}
							ImGui::EndCombo();
						}
						break;
					}
					case GraphicsMode::DX:
					{
						DXRenderManager* renderManagerDX = dynamic_cast<DXRenderManager*>(renderManager);

						//DXTexture* objTex = renderManagerDX->GetTexture(sprite->GetTextureName().c_str());
						//if(objTex != nullptr)
						//{
						//	VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(*objTex->GetSampler(), *objTex->GetImageView(), DX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
						//	ImGui::Image((ImTextureID)descriptorSet, ImVec2(64, 64), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
						//	ImGui::SameLine();
						//}

						if (ImGui::BeginCombo("TextureList", sprite->GetTextureName().c_str()))
						{
							for (auto& tex : renderManagerDX->GetTextures())
							{
								int index = 0;
								const bool is_selected = tex->GetName().c_str() == sprite->GetTextureName().c_str();

								//VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(*tex->GetSampler(), *tex->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
								//ImGui::Image((ImTextureID)descriptorSet, ImVec2(16, 16), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
								//ImGui::SameLine();

								if (ImGui::Selectable(tex->GetName().c_str()))
								{
									sprite->ChangeTexture(tex->GetName());
								}
								if (is_selected)
								{
									ImGui::SetItemDefaultFocus();
								}
								index++;
							}
							ImGui::EndCombo();
						}
						break;
					}
					}
				}
				else
				{
					sprite->SetIsTex(false);
				}
				sprite->SetIsTex(isTextureOn);
				ImGui::Spacing();
			}

			if (ImGui::BeginMenu("Select Mesh Type"))
			{
				if (ImGui::MenuItem("Plane", "0"))
				{
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->DeleteComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->AddComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
						{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::PLANE, "", 2, 2, color, metallic, roughness); });
				}
				if (ImGui::MenuItem("Cube", "1"))
				{
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->DeleteComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->AddComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
						{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::CUBE, "", 2, 2, color, metallic, roughness); });
				}
				if (ImGui::MenuItem("Sphere", "2"))
				{
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->DeleteComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->AddComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
						{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::SPHERE, "", 30, 30, color, metallic, roughness); });
				}
				if (ImGui::MenuItem("Torus", "3"))
				{
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->DeleteComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->AddComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
						{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::TORUS, "", 15, 15, color, metallic, roughness); });
				}
				if (ImGui::MenuItem("Cylinder", "4"))
				{
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->DeleteComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->AddComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
						{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::CYLINDER, "", 10, 10, color, metallic, roughness); });
				}
				if (ImGui::MenuItem("Cone", "5"))
				{
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->DeleteComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
						{ obj->AddComponent<DynamicSprite>(); });
					Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
						{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::CONE, "", 10, 10, color, metallic, roughness); });
				}
				if (ImGui::MenuItem("Obj Model", "6"))
				{
					isSelectedObjModel = true;
				}
				ImGui::EndMenu();
			}

			if (isSelectedObjModel)
			{
				ImGui::OpenPopup("Select Model");
				isShowPopup = true;
			}
			ImGui::Spacing();
		}
	}
	else
	{
	}
	spriteComp = nullptr;
	renderManager = nullptr;
}

void ObjectManager::LightControllerForImGui(Light* light)
{
	if (light->GetLightType() != LightType::NONE)
	{
		glm::vec3 position = light->GetPosition();
		glm::vec4 rotation = light->GetRotate();
		glm::vec4 color = light->GetColor();
		float radius = light->GetRadius();
		float intensity = light->GetIntensity();

		float ambient = light->GetAmbientStrength();
		float specular = light->GetSpecularStrength();


		if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::ColorEdit4("Light Color", &color.r))
			{
				light->SetColor(color);
			}
			if (ImGui::DragFloat("radius", &radius, 0.1f, 0.f, 100.f))
			{
				light->SetRadius(radius);
			}
			if (ImGui::DragFloat("intensity", &intensity, 0.1f, 0.f, 100.f))
			{
				light->SetIntensity(intensity);
			}

			if (light->GetLightType() == LightType::DIRECTIONAL)
			{
				ImGui::DragFloat3("Light Direction", &position.x, 0.01f);
				ImGui::Spacing();

				light->SetXPosition(position.x);
				light->SetYPosition(position.y);
				light->SetZPosition(position.z);
			}
			if (light->GetLightType() == LightType::POINT)
			{
				float constant = light->GetConstant();
				float linear = light->GetLinear();
				float quadratic = light->GetQuadratic();

				ImGui::DragFloat3("Light Position", &position.x, 0.01f);
				ImGui::Spacing();
				//ImGui::DragFloat3("Light Rotation", &rotation.x, 0.5f);

				if (ImGui::SliderFloat("constant", &constant, 0.f, 1.f))
				{
					light->SetConstant(constant);
				}
				if (ImGui::SliderFloat("linear", &linear, 0.f, 1.f))
				{
					light->SetLinear(linear);
				}
				if (ImGui::SliderFloat("quadratic", &quadratic, 0.f, 1.f))
				{
					light->SetQuadratic(quadratic);
				}

				light->SetXPosition(position.x);
				light->SetYPosition(position.y);
				light->SetZPosition(position.z);
				/*if(light->GetRotate() != rotation)
				{
					light->SetRotate(rotation);
				}*/
			}

			ImGui::SliderFloat("Ambient Strength", &ambient, 0.f, 1.f);
			ImGui::SliderFloat("Specular Strength", &specular, 0.f, 1.f);

			light->SetAmbientStrength(ambient);
			light->SetSpecularStrength(specular);
		}
	}
	else
	{
		ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		ImVec2 popupSize = ImVec2(200, 300);
		ImVec2 popupPos = ImVec2((displaySize.x - popupSize.x) / 2.0f, (displaySize.y - popupSize.y) / 2.0f);

		ImGui::SetNextWindowPos(popupPos, ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(popupSize);

		if (!ImGui::IsPopupOpen("Select Light Type"))
		{
			isShowPopup = true;
			ImGui::OpenPopup("Select Light Type");
		}

		if (ImGui::BeginPopupModal("Select Light Type", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::Button("Directional"))
			{
				isShowPopup = false;
				light->AddLight(LightType::DIRECTIONAL, 1.f);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("Point"))
			{
				isShowPopup = false;
				light->AddLight(LightType::POINT, 1.f);
				ImGui::CloseCurrentPopup();
			}

			ImGui::Separator();
			ImGui::EndPopup();
		}

		ImGui::SetNextWindowBgAlpha(0.5f);
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("Background Overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
		ImGui::End();
	}
}

void ObjectManager::SkeletalAnimatorControllerForImGui(SkeletalAnimator* animator)
{
	if (!animator) return;

	if (ImGui::CollapsingHeader("Skeletal Animator"))
	{
		ImGui::Text("Animation Controls");

		// Playback Controls
		if (ImGui::Button("Play")) 
		{
			animator->Play();
		}
		ImGui::SameLine();

		if (ImGui::Button("Pause")) 
		{
			animator->Pause();
		}
		ImGui::SameLine();

		if (ImGui::Button("Stop")) 
		{
			animator->Stop();
		}

		// Parameters
		float speed = animator->GetSpeed();
		if (ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 5.0f))
		{
			animator->SetSpeed(speed);
		}

		// Debug Toggle
		ImGui::Checkbox("Show Bones", &isShowBone);

		// Information
		SkeletalAnimation* anim = animator->GetCurrentAnimation();
		if (anim)
		{
			float currentTime = animator->GetCurrentAnimationTime();
			float duration = anim->GetDuration();
			ImGui::SliderFloat("Time", &currentTime, 0.0f, duration, "%.2f");
			animator->SetCurrentAnimationTime(currentTime);
		}
		else
		{
			ImGui::Text("No Animation Playing");
		}

		Object* currentObj = FindObjectWithId(currentIndex);
		if (isShowBone && currentObj->HasComponent<SkeletalAnimator>())
		{
			SkeletalAnimator* animatorComp = currentObj->GetComponent<SkeletalAnimator>();
			SkeletalAnimation* currentAnim = animatorComp->GetCurrentAnimation();
			if (currentAnim)
			{
				const auto& transforms = animatorComp->GetGlobalBoneTransforms();

				// Build model matrix identical to DynamicSprite::UpdateModel (GL mode)
				glm::vec3 pos = currentObj->GetPosition();
				glm::vec3 rot = currentObj->GetRotate3D();
				glm::vec3 scale = currentObj->GetSize();

				glm::mat4 rotationMatrix = glm::toMat4(glm::quat(glm::radians(-rot)));
				glm::mat4 model = glm::translate(glm::mat4(1.0f), pos)
					* rotationMatrix
					* glm::scale(glm::mat4(1.0f), scale);

				// No globalInverseTransform needed — bones and vertices share the same coordinate space
				RenderBoneHierarchy(&currentAnim->GetRootNode(), transforms, model);
			}
		}
	}
}

void ObjectManager::AnimationStateMachineControllerForImGui(SkeletalAnimationStateMachine* fsm)
{
	if (!fsm) return;

	if (ImGui::CollapsingHeader("Animation State Machine"))
	{
		ImGui::Text("Current State: %s", fsm->GetCurrentStateName().c_str());

		std::vector<std::string> states = fsm->GetStateNames();
		static int selectedStateIndex = 0;

		if (states.empty()) return;

		// Combo box to select state
		if (ImGui::BeginCombo("States", states[selectedStateIndex].c_str()))
		{
			for (int i = 0; i < states.size(); i++)
			{
				bool isSelected = (selectedStateIndex == i);
				if (ImGui::Selectable(states[i].c_str(), isSelected))
				{
					selectedStateIndex = i;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Transition"))
		{
			fsm->ChangeState(states[selectedStateIndex]);
		}
	}
}

void ObjectManager::RenderBoneHierarchy(const AssimpNodeData* node, const std::map<std::string, glm::mat4>& animatedTransforms, glm::mat4 objectTransform)
{
	// 1. 현재 노드의 뼈 행렬 가져오기
	if (animatedTransforms.find(node->name) == animatedTransforms.end())
	{
		// 맵에 없는 노드(더미 등)는 그냥 자식으로 통과시킵니다.
		for (const auto& child : node->children)
		{
			RenderBoneHierarchy(&child, animatedTransforms, objectTransform);
		}
		return;
	}

	// 2. 최종 월드 좌표 계산
	glm::mat4 nodeGlobalMatrix = animatedTransforms.at(node->name);
	glm::mat4 nodeWorldMatrix = objectTransform * nodeGlobalMatrix;

	// 3. 화면 좌표 변환
	glm::mat4 view = Engine::GetCameraManager().GetViewMatrix();
	glm::mat4 proj = Engine::GetCameraManager().GetProjectionMatrix();
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();

	glm::vec3 currentPos = glm::vec3(nodeWorldMatrix[3]);
	glm::vec2 screenPos = WorldToScreen(currentPos, view, proj);

	// [디버깅] 뼈 위치 정보 출력
	static bool debugPrint = true;
	if (debugPrint && node->name.find("Armature") == std::string::npos)
	{
		char debugBuffer[256];
		snprintf(debugBuffer, sizeof(debugBuffer),
			"Bone: %s | GlobalPos: (%.2f, %.2f, %.2f) | ScreenPos: (%.2f, %.2f)",
			node->name.c_str(),
			currentPos.x, currentPos.y, currentPos.z,
			screenPos.x, screenPos.y);
		Engine::GetLogger().LogDebug(LogCategory::Engine, debugBuffer);
	}

	// 4. 조인트(점) 그리기
	if (screenPos.x != -1 && screenPos.y != -1)
	{
		ImU32 color = IM_COL32(0, 255, 0, 255);
		if (node->name == selectedBoneName) color = IM_COL32(255, 0, 0, 255);

		drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 5.0f, color);
	}

	// 5. 자식 노드 처리
	for (const auto& child : node->children)
	{
		// 자식 뼈가 맵에 존재할 때만 선을 그립니다.
		if (animatedTransforms.find(child.name) != animatedTransforms.end())
		{
			glm::mat4 childGlobalMatrix = animatedTransforms.at(child.name);
			glm::mat4 childWorldMatrix = objectTransform * childGlobalMatrix;

			glm::vec3 childPos = glm::vec3(childWorldMatrix[3]);
			glm::vec2 childScreenPos = WorldToScreen(childPos, view, proj);

			// 두 점이 모두 유효한 경우만 선 그리기
			if (screenPos.x >= 0 && screenPos.y >= 0 &&
				childScreenPos.x >= 0 && childScreenPos.y >= 0)
			{
				drawList->AddLine(
					ImVec2(screenPos.x, screenPos.y),
					ImVec2(childScreenPos.x, childScreenPos.y),
					IM_COL32(255, 255, 0, 255),
					2.0f);
			}
		}

		// 재귀 호출
		RenderBoneHierarchy(&child, animatedTransforms, objectTransform);
	}
}

void ObjectManager::SelectObjectWithMouse()
{
	//SelectObject
	if (Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::LEFT) && isShowPopup == false)
	{
		Ray ray = Engine::GetCameraManager().CalculateRayFrom2DPosition(Engine::GetInputManager().GetMousePosition());
		if (isDragObject == false)
		{
			float tMin, tMax;
			float closestDistance = std::numeric_limits<float>::max();

			for (auto& object : objectMap)
			{
				glm::vec3 posTemp = object.second.get()->GetPosition();
				glm::vec3 sizeTemp = object.second.get()->GetSize();

				glm::vec3 invDir = 1.0f / ray.direction;
				glm::vec3 t0 = (posTemp - (sizeTemp / 2.f) - ray.origin) * invDir;
				glm::vec3 t1 = (posTemp + (sizeTemp / 2.f) - ray.origin) * invDir;

				glm::vec3 tMinVec = glm::min(t0, t1);
				glm::vec3 tMaxVec = glm::max(t0, t1);

				tMin = glm::max(glm::max(tMinVec.x, tMinVec.y), tMinVec.z);
				tMax = glm::min(glm::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

				if (tMax >= tMin && tMax >= 0)
				{
					if (tMin < closestDistance)
					{
						closestDistance = tMin;
						closestObjectId = object.second.get()->GetId();
						isDragObject = true;
					}
				}
			}
		}
		else
		{
			if (currentIndex == closestObjectId)
			{
				Object* objT = FindObjectWithId(closestObjectId);

				if (isObjGravityOn == false && objT->HasComponent<Physics3D>() && objT->GetComponent<Physics3D>()->GetIsGravityOn() == true)
				{
					objT->GetComponent<Physics3D>()->SetIsGravityOn(false);
					isObjGravityOn = true;
				}
				ray = Engine::GetCameraManager().CalculateRayFrom2DPosition(Engine::GetInputManager().GetMousePosition());
				glm::vec3 planeNormal = Engine::GetCameraManager().GetBackVector();
				glm::vec3 objectPosition = objT->GetPosition();
				float distanceToPlane = glm::dot(planeNormal, objectPosition - ray.origin) / glm::dot(planeNormal, ray.direction);
				glm::vec3 intersectionPoint = ray.origin + distanceToPlane * ray.direction;

				//objT->SetPosition(intersectionPoint);
				objT = nullptr;
			}
			else
			{
				currentIndex = closestObjectId;
			}
		}
	}
	else if (isDragObject == true && !Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::LEFT) && isShowPopup == false)
	{
		if (isObjGravityOn == true)
		{
			Object* objT = FindObjectWithId(closestObjectId);
			if (objT->HasComponent<Physics3D>())
			{
				objT->GetComponent<Physics3D>()->SetIsGravityOn(true);
				isObjGravityOn = false;
			}
			objT = nullptr;
		}
		closestObjectId = 0;
		isDragObject = false;
	}
	//SelectObject
}

void ObjectManager::AddComponentPopUpForImGui()
{
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImVec2 popupSize = ImVec2(400, 300);
	ImVec2 popupPos = ImVec2((displaySize.x - popupSize.x) / 2.0f, (displaySize.y - popupSize.y) / 2.0f);

	ImGui::SetNextWindowPos(popupPos, ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(popupSize);

	if (ImGui::BeginPopupModal("Select Component", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, 3 * ImGui::GetTextLineHeightWithSpacing()));
		ImGui::BeginChild("Scrolling");
		Object* currentObj = FindObjectWithId(currentIndex);

		for (int i = 0; i < static_cast<int>(ComponentTypes::INVALID); i++)
		{
			if (ImGui::Selectable(ComponentToString(static_cast<ComponentTypes>(i)).c_str(), selectedItem == i))
			{
				selectedItem = i;

				switch (static_cast<ComponentTypes>(i))
				{
				case ComponentTypes::SPRITE:
					if (currentObj->HasComponent<DynamicSprite>() == false)
					{
						currentObj->AddComponent<DynamicSprite>();
						currentObj->GetComponent<DynamicSprite>()->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1);
						//TBD
					}
					break;
				case ComponentTypes::PHYSICS2D:
					if (currentObj->HasComponent<Physics2D>() == false)
					{
						currentObj->AddComponent<Physics2D>();
					}
					break;
				case ComponentTypes::PHYSICS3D:
					if (currentObj->HasComponent<Physics3D>() == false)
					{
						currentObj->AddComponent<Physics3D>();
					}
					break;
				case ComponentTypes::LIGHT:
					if (currentObj->HasComponent<Light>() == false)
					{
						currentObj->AddComponent<Light>();
					}
					break;
				}
				isShowPopup = false;
				ImGui::CloseCurrentPopup();
			}
		}
		currentObj = nullptr;
		ImGui::EndChild();

		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button("Close"))
		{
		 isShowPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin("Background Overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
	ImGui::End();
}

void ObjectManager::SelectObjModelPopUpForImGui()
{
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImVec2 popupSize = ImVec2(200, 300);
	ImVec2 popupPos = ImVec2((displaySize.x - popupSize.x) / 2.0f, (displaySize.y - popupSize.y) / 2.0f);

	ImGui::SetNextWindowPos(popupPos, ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(popupSize);

	if (ImGui::BeginPopupModal("Select Model", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, 4 * ImGui::GetTextLineHeightWithSpacing()));
		ImGui::BeginChild("Scolling");
		Object* currentObj = FindObjectWithId(currentIndex);
		glm::vec4 color = currentObj->GetComponent<DynamicSprite>()->GetColor();

		//for (int i = 0; i < 9; i++)
		{
			if (ImGui::Selectable("Cube"))
			{
				isShowPopup = false;

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", stacks, slices, color, metallic, roughness); });
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Car"))
			{
				isShowPopup = false;

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::OBJ, "../Game/assets/Models/car.obj", stacks, slices, color, metallic, roughness); });
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Diamond"))
			{
				isShowPopup = false;

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::OBJ, "../Game/assets/Models/diamond.obj", stacks, slices, color, metallic, roughness); });
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Dodecahedron"))
			{
				isShowPopup = false;

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::OBJ, "../Game/assets/Models/dodecahedron.obj", stacks, slices, color, metallic, roughness); });
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Gourd"))
			{
				isShowPopup = false;

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::OBJ, "../Game/assets/Models/gourd.obj", stacks, slices, color, metallic, roughness); });
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Sphere"))
			{
				isShowPopup = false;

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::OBJ, "../Game/assets/Models/sphere.obj", stacks, slices, color, metallic, roughness); });
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Teapot"))
			{
				isShowPopup = false;

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::OBJ, "../Game/assets/Models/teapot.obj", stacks, slices, color, metallic, roughness); });
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Vase"))
			{
				isShowPopup = false;

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::OBJ, "../Game/assets/Models/vase.obj", stacks, slices, color, metallic, roughness); });
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Monkey"))
			{
				isShowPopup = false;

				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->DeleteComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [](Object* obj)
					{ obj->AddComponent<DynamicSprite>(); });
				Engine::GetObjectManager().QueueObjectFunction(currentObj, [=](Object* obj)
					{ obj->GetComponent<DynamicSprite>()->CreateMesh3D(MeshType::OBJ, "../Game/assets/Models/monkey.obj", stacks, slices, color, metallic, roughness); });
				ImGui::CloseCurrentPopup();
			}
		}
		currentObj = nullptr;
		ImGui::EndChild();
		ImGui::Separator();

		if (ImGui::Button("Close"))
		{
		 isShowPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin("Background Overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
	ImGui::End();
}
