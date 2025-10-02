//Author: DOYEONG LEE
//Project: CubeEngine
//File: Timer.hpp

#pragma once
#include <chrono>
#include <deque>
#include <vector>    
#include <numeric>   
#include <algorithm> 
#include "imgui.h"

// 1 / 120 = 0.008333
// 1 / 60 = 0.016667

enum class FrameRate
{
	UNLIMIT = 1,
	FPS_30 = 30,
	FPS_60 = 60,
	FPS_120 = 120,
	FPS_144 = 144,
	FPS_240 = 240,
	FPS_300 = 300
};

class Timer
{
public:
	void Init(FrameRate rate = FrameRate::FPS_60)
	{
		frame = rate;
		timeStamp = clock::now();
		lastTimeStamp = clock::now();
		fpsCalcTime = lastTimeStamp;

		if (frame != FrameRate::UNLIMIT)
		{
			framePerTime = 1.f / static_cast<float>(frame);
		}
		else
		{
			framePerTime = 0.f;
		}

		ResetHistory();
	}

	void Update() { timeStamp = clock::now(); }

	void AddFrameHistory(float amount)
	{
		UpdateHistory(amount);
	}

	void Reset()
	{
		timeStamp = clock::now();
		lastTimeStamp = timeStamp;
	}

	void ResetLastTimeStamp() noexcept
	{
		lastTimeStamp = timeStamp;
	}

	void ResetFPSCalculateTime() noexcept
	{
		fpsCalcTime = timeStamp;
	}

	void ShowFpsGraph()
	{
		std::vector<float> data(fpsHistory.begin(), fpsHistory.end());
		ImVec2 pos = ImVec2(ImGui::GetMainViewport()->Size.x, 16.f);
		ImVec2 pivot = ImVec2(1.0f, 0.0f);

		ImGui::SetNextWindowBgAlpha(0.35f);
		ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pivot);
		ImGui::SetNextWindowSize(ImVec2(350.0f, 110.0f), ImGuiCond_Always);
		ImGui::Begin("FPS History", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoSavedSettings);

		float avgFps = 0.0f;
		float minFps = 0.0f;
		float maxFps = 0.0f;
		float graphMin = 0.0f;
		float graphMax = 150.0f;

		if (!data.empty())
		{
			float total_fps = std::accumulate(data.begin(), data.end(), 0.0f);
			avgFps = total_fps / data.size();

			std::vector<float> sorted_data = data;
			std::sort(sorted_data.begin(), sorted_data.end());
			minFps = sorted_data.front();
			maxFps = sorted_data.back();

			const float padding = 10.0f;
			graphMin = minFps - padding;
			graphMax = maxFps + padding;
			if (graphMin < 0.0f) {
				graphMin = 0.0f;
			}
		}

		char text[32];
		sprintf_s(text, "FPS: %.1f", currentFps);
		ImGui::PlotLines(
			"##fps",
			data.data(),
			(int)data.size(),
			0,
			text,
			graphMin,
			graphMax,
			ImVec2(ImGui::GetContentRegionAvail().x, 50)
		);

		ImGui::Text("Min: %.1f, Max: %.1f, Avg: %.1f", minFps, maxFps, avgFps);
		ImGui::End();
	}

	float GetDeltaTime() const noexcept
	{
		return std::chrono::duration_cast<second>(timeStamp - lastTimeStamp).count();
	}

	float GetFrameRateCalculateTime() const noexcept
	{
		return std::chrono::duration_cast<second>(timeStamp - fpsCalcTime).count();
	}

	FrameRate GetFrameRate() { return frame; }
	float GetFramePerTime() { return framePerTime; }

private:
	void UpdateHistory(float fps)
	{
		currentFps = fps;
		fpsHistory.push_back(fps);
		if (fpsHistory.size() > 100)
		{
			fpsHistory.pop_front();
		}
	}

	void ResetHistory()
	{
		fpsHistory.clear();
		currentFps = 0.0f;
	}

	using clock = std::chrono::steady_clock;
	using second = std::chrono::duration<float>;

	std::chrono::steady_clock::time_point timeStamp;
	std::chrono::steady_clock::time_point lastTimeStamp;
	std::chrono::steady_clock::time_point fpsCalcTime;

	FrameRate frame = FrameRate::FPS_60;
	std::deque<float> fpsHistory;
	float framePerTime = 0.f;
	float currentFps = 0.0f;
};