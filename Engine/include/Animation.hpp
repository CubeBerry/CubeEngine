//Author: JEYOON YU
//Project: CubeEngine
//File: Animation.hpp
#pragma once
#include <filesystem>
#include <vector>

class Animation
{
public:
	Animation() = default;
	Animation(const std::filesystem::path& fileName);
	~Animation();
	void Update(double dt);
	int GetDisplayFrame();
	void ResetAnimation();
	bool IsAnimationDone();

private:
	enum class Command
	{
		PlayFrame,
		Loop,
		End,
	};

	class CommandData
	{
	public:
		virtual ~CommandData() {}
		virtual Command GetType() = 0;
	};

	class PlayFrame : public CommandData
	{
	public:
		PlayFrame(int frame, double duration);
		virtual Command GetType() override { return Command::PlayFrame; }
		void Update(double dt);
		bool IsFrameDone();
		void ResetTime();
		int GetFrameNum();
	private:
		int frame;
		double targetTime;
		double timer;
	};

	class Loop : public CommandData
	{
	public:
		Loop(int loopToIndex);
		virtual Command GetType() override { return Command::Loop; }
		int GetLoopToIndex();
	private:
		int loopToIndex;
	};

	class End : public CommandData
	{
	public:
		virtual Command GetType() override { return Command::End; }
	};

	bool isAnimationDone;
	int animSequenceIndex;
	PlayFrame* currPlayFrameData;
	std::vector<CommandData*> animations;
};