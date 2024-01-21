//Author: JEYOON YU
//Project: CubeEngine
//File: Animation.cpp
#include <fstream>
#include "Engine.hpp"
#include "Animation.hpp"

Animation::Animation(const std::filesystem::path& fileName) : animSequenceIndex(0)
{
	if (fileName.extension() != ".anm")
	{
		throw std::runtime_error("Bad Filetype.  " + fileName.generic_string() + " not a sprite info file (.anm)");
	}
	std::ifstream inFile(fileName);
	if (inFile.is_open() == false)
	{
		throw std::runtime_error("Failed to load " + fileName.generic_string());
	}

	std::string label;
	while (inFile.eof() == false)
	{
		inFile >> label;
		if (label == "PlayFrame")
		{
			int frame;
			float targetTime;
			inFile >> frame;
			inFile >> targetTime;

			animations.push_back(new PlayFrame(frame, targetTime));
		}
		else if (label == "Loop")
		{
			int loopToFrame;
			inFile >> loopToFrame;
			animations.push_back(new Loop(loopToFrame));
		}
		else if (label == "End")
		{
			animations.push_back(new End());
		}
		else {
			//Engine::GetLogger().LogError("Unknown command " + label + " in anm file " + fileName.generic_string());
		}
	}
	ResetAnimation();
}

Animation::~Animation()
{
	for (CommandData* command : animations)
	{
		delete command;
	}
	animations.clear();
}

void Animation::Update(double dt)
{
	currPlayFrameData->Update(dt);
	if (currPlayFrameData->IsFrameDone() == true)
	{
		currPlayFrameData->ResetTime();
		++animSequenceIndex;
		if (animations[animSequenceIndex]->GetType() == Command::PlayFrame)
		{
			currPlayFrameData = static_cast<PlayFrame*>(animations[animSequenceIndex]);
		}
		else if (animations[animSequenceIndex]->GetType() == Command::Loop)
		{
			Loop* loopData = static_cast<Loop*>(animations[animSequenceIndex]);
			animSequenceIndex = loopData->GetLoopToIndex();
			if (animations[animSequenceIndex]->GetType() == Command::PlayFrame)
			{
				currPlayFrameData = static_cast<PlayFrame*>(animations[animSequenceIndex]);
			}
			else
			{
				//Engine::GetLogger().LogError("Loop does not go to PlayFrame");
				ResetAnimation();
			}
		}
		else if (animations[animSequenceIndex]->GetType() == Command::End)
		{
			isAnimationDone = true;
			return;
		}
	}
}

int Animation::GetDisplayFrame()
{
	return currPlayFrameData->GetFrameNum();
}

void Animation::ResetAnimation()
{
	animSequenceIndex = 0;
	currPlayFrameData = static_cast<PlayFrame*>(animations[animSequenceIndex]);
	isAnimationDone = false;
}

bool Animation::IsAnimationDone()
{
	return isAnimationDone;
}

Animation::PlayFrame::PlayFrame(int frame, double duration)
	: frame(frame), targetTime(duration), timer(0) {}

void Animation::PlayFrame::Update(double dt)
{
	timer += dt;
}

bool Animation::PlayFrame::IsFrameDone()
{
	if (timer >= targetTime)
	{
		return true;
	}
	return false;
}

void Animation::PlayFrame::ResetTime()
{
	timer = 0;
}

int Animation::PlayFrame::GetFrameNum()
{
	return frame;
}

Animation::Loop::Loop(int loopToIndex) : loopToIndex(loopToIndex) {}

int Animation::Loop::GetLoopToIndex()
{
	return loopToIndex;
}