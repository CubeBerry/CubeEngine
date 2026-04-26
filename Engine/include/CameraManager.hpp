//Author: DOYEONG LEE
//Project: CubeEngine
//File: CameraManager.hpp
#pragma once
#include "Camera.hpp"

#include <vector>
#include <memory>

class Object;
class CameraManager
{
public:
	CameraManager() {};
	~CameraManager();

	void Init(glm::vec2 viewSize, CameraType type = CameraType::TwoDimension, float zoom = 45.f, float angle = 0.f);
	void Update(); 
	void DrawCameraDebug();
	void Reset();
	void DeleteAllCameras();

	Camera* AddCamera(CameraType type, std::string name = "");
	void DeleteCamera(int index);
	void SetMainCamera(int index);
	int GetMainCameraIndex() const { return mainCameraIndex; }

	Camera* GetCamera(int index = 0);
	size_t GetCameraCount() const;

	void ControlCamera(float dt);
	bool IsInCamera(Object* object, int index = 0);
	bool IsScreenPointOccluded(glm::vec2 screenPos, int cameraIndex);

	void CameraControllerImGui();
	int currentObjIndex = 0;
private:
	std::vector<std::unique_ptr<Camera>> cameras;
	int mainCameraIndex = 0;
	int selectedCameraIndex = 0;
	int maxCameraIndex = 0;
};
