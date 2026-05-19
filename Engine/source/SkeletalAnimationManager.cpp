//Author: DOYEONG LEE
//Project: CubeEngine
//File: SkeletalAnimationManager.cpp
#include "SkeletalAnimationManager.hpp"
#include "BasicComponents/SkeletalAnimator.hpp"
#include "Engine.hpp"

#include <algorithm>

void SkeletalAnimationManager::AddAnimator(SkeletalAnimator* animator)
{
	std::lock_guard<std::mutex> lock(registryMutex);
	if (std::find(animators.begin(), animators.end(), animator) == animators.end())
	{
		animators.push_back(animator);
	}
}

void SkeletalAnimationManager::RemoveAnimator(SkeletalAnimator* animator)
{
	std::lock_guard<std::mutex> lock(registryMutex);
	auto it = std::find(animators.begin(), animators.end(), animator);
	if (it != animators.end())
	{
		animators.erase(it);
	}
}

void SkeletalAnimationManager::Update(float dt)
{
	if (animators.empty())
	{
		return;
	}

	// Each animator's bone calculation is independent — safe to parallelize
	auto handle = Engine::GetJobSystem().QueueParallelWork(
		static_cast<uint32_t>(animators.size()),
		[this, dt](uint32_t begin, uint32_t end)
		{
			for (uint32_t i = begin; i < end; ++i)
			{
				// CPU bone transform calculation
				animators[i]->UpdateBoneTransforms(dt);

				// Queue GPU upload to be flushed on main thread
				animators[i]->QueueGPUBoneUpload();
			}
		},
		1 // One animator per batch — each is independent
	);
	Engine::GetJobSystem().WaitForWork(handle);
}
