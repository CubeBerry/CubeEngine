//Author: DOYEONG LEE
//Project: CubeEngine
//File: Timer.hpp
#pragma once
#include <chrono>
//1 / 120 = 0.008333
//1 / 60 = 0.016667

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
		framePerTime = 1.f / static_cast<float>(frame);
	}

	void Update() { timeStamp = clock::now();}

	float Update(double& fps, double fpsCalc)
	{
		static auto prevTime = clock::now();
		auto currTime = clock::now();

		std::chrono::duration<float> deltaTime = currTime - prevTime;
		prevTime = currTime;

		static float count = 0.0f;
		static auto startTime = clock::now();
		std::chrono::duration<float> elapsedTime = currTime - startTime;
		++count;

		fpsCalc = (fpsCalc < 0.0f) ? 0.0f : fpsCalc;
		fpsCalc = (fpsCalc > 10.0f) ? 10.0f : fpsCalc;

		if (elapsedTime.count() >= fpsCalc) 
		{
			fps = count / elapsedTime.count();
			startTime = currTime;
			count = 0.0f;
		}

		return deltaTime.count();
	}

	void ResetLastTimeStamp() noexcept
	{
		lastTimeStamp = timeStamp;
	}

	void ResetFPSCalculateTime() noexcept
	{
		fpsCalcTime = timeStamp;
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
	using clock = std::chrono::system_clock;
	using second = std::chrono::duration <float>;

	std::chrono::system_clock::time_point timeStamp;
	std::chrono::system_clock::time_point lastTimeStamp;
	std::chrono::system_clock::time_point fpsCalcTime;

	FrameRate frame = FrameRate::FPS_60;
	float framePerTime = 0.f;
};
