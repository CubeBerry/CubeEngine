//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimationManager.hpp
#pragma once

#include <vector>
#include <mutex>

class SkeletalAnimator;

class SkeletalAnimationManager
{
public:
	SkeletalAnimationManager()  = default;
	~SkeletalAnimationManager() = default;

	// Called from SkeletalAnimator::Init() / End()
	void AddAnimator(SkeletalAnimator* animator);
	void RemoveAnimator(SkeletalAnimator* animator);

	// Runs bone matrix calculation in parallel via JobSystem,
	// then queues GPU upload for each animator
	void Update(float dt);

private:
	std::vector<SkeletalAnimator*> animators;
	std::mutex registryMutex; // Protects Register / Unregister
};
