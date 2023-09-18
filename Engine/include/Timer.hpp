#pragma once
#include <chrono>
#include <iostream>

//1 / 120 = 0.008333
//1 / 60 = 0.016667

enum FrameRate
{
	FPS_30 = 30,
	FPS_60 = 60,
	FPS_120 = 120,
	FPS_144 = 144
};

class Timer
{
public:
	void Init(FrameRate rate = FrameRate::FPS_60)
	{
		frame = rate;
		timeStamp = clock::now();
		lastTimeStamp = timeStamp;
		fpsCalcTime = lastTimeStamp;
	}

	void Update() { timeStamp = clock::now(); }

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
		return std::chrono::duration_cast<second>(clock::now() - lastTimeStamp).count();
	}

	float GetFrameRateCalculateTime() const noexcept
	{
		return std::chrono::duration_cast<second>(timeStamp - fpsCalcTime).count();
	}

	FrameRate GetFrameRate() { return frame; }

private:
	using clock = std::chrono::system_clock;
	using second = std::chrono::duration <float>;

	std::chrono::time_point <clock> timeStamp;
	std::chrono::time_point <clock> lastTimeStamp;
	std::chrono::time_point <clock> fpsCalcTime;

	FrameRate frame = FrameRate::FPS_60;
};
