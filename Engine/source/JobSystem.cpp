//Author: DOYEONG LEE
//Project: CubeEngine
//File: JobSystem.cpp
#include "Engine.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "DXRenderManager.hpp"

#include <algorithm>

void JobQueue::Push(Job job)
{
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		jobs.push_back(std::move(job));
	}
	cv.notify_one();
}

std::optional<Job> JobQueue::TryPop()
{
	std::lock_guard<std::mutex> lock(queueMutex);
	if (jobs.empty())
	{
		return std::nullopt;
	}

	Job job = std::move(jobs.front());
	jobs.pop_front();
	return job;
}

std::optional<Job> JobQueue::WaitAndPop(std::atomic<bool>& running)
{
	std::unique_lock<std::mutex> lock(queueMutex);
	cv.wait(lock, [&]
	{
		return !running.load(std::memory_order_relaxed) || !jobs.empty();
	});

	if (!running.load(std::memory_order_relaxed) && jobs.empty())
	{
		return std::nullopt;
	}

	Job job = std::move(jobs.front());
	jobs.pop_front();
	return job;
}

void JobQueue::NotifyAll()
{
	cv.notify_all();
}

void JobSystem::Start(uint32_t threadCount)
{
	if (running.load())
	{
		return;
	}

	if (threadCount == 0)
	{
		threadCount = std::max(1u, std::thread::hardware_concurrency() - 1);
	}

	running = true;
	workerThreads.reserve(threadCount);
	for (uint32_t i = 0; i < threadCount; ++i)
	{
		workerThreads.emplace_back(&JobSystem::WorkerLoop, this, i);
	}

	Engine::GetLogger().LogDebug(LogCategory::Engine,
		"Job System Initialized (" + std::to_string(threadCount) + " workers)");
}

void JobSystem::Stop()
{
	if (!running.load())
	{
		return;
	}

	running = false;
	// Wake all workers so they can exit their loop
	jobQueue.NotifyAll();

	for (auto& t : workerThreads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	workerThreads.clear();

	Engine::GetLogger().LogDebug(LogCategory::Engine, "Job System Stop");
}

JobHandle JobSystem::QueueWork(std::function<void()> task)
{
	auto counter = std::make_shared<std::atomic<int>>(1);
	jobQueue.Push(Job{ std::move(task), counter });

	JobHandle handle;
	handle.counter = counter;
	return handle;
}

JobHandle JobSystem::QueueParallelWork(
	uint32_t count,
	std::function<void(uint32_t, uint32_t)> body,
	uint32_t batchSize)
{
	if (count == 0)
	{
		return JobHandle{};
	}

	uint32_t batchCount = (count + batchSize - 1) / batchSize;
	auto counter = std::make_shared<std::atomic<int>>(static_cast<int>(batchCount));

	for (uint32_t b = 0; b < batchCount; ++b)
	{
		uint32_t begin = b * batchSize;
		uint32_t end = std::min(begin + batchSize, count);
		jobQueue.Push(Job{
			[body, begin, end]()
			{
				body(begin, end);
			},
			counter
		});
	}

	JobHandle handle;
	handle.counter = counter;
	return handle;
}

void JobSystem::WaitForWork(const JobHandle& handle)
{
	// Main thread helps execute jobs while waiting for the target job
	while (!handle.IsComplete())
	{
		if (!ExecuteOneJob())
		{
			std::this_thread::yield();
		}
	}
}

void JobSystem::WaitForAll(const std::vector<JobHandle>& handles)
{
	// Wait for each handle, helping execute jobs in between
	bool allComplete = false;
	while (!allComplete)
	{
		allComplete = true;
		for (const auto& h : handles)
		{
			if (!h.IsComplete())
			{
				allComplete = false;
				break;
			}
		}

		if (!allComplete)
		{
			if (!ExecuteOneJob())
			{
				std::this_thread::yield();
			}
		}
	}
}

void JobSystem::WorkerLoop(uint32_t /*workerIndex*/)
{
	while (running.load(std::memory_order_relaxed))
	{
		auto item = jobQueue.WaitAndPop(running);
		if (!item.has_value())
		{
			continue;
		}

		// Store counter before executing — ensures decrement even if task throws
		auto counter = item->counter;
		try
		{
			item->task();
		}
		catch (...)
		{
			Engine::GetLogger().LogError(LogCategory::Engine, "Job threw an exception");
		}
		counter->fetch_sub(1, std::memory_order_release);
	}
}

bool JobSystem::ExecuteOneJob()
{
	auto item = jobQueue.TryPop();
	if (!item.has_value())
	{
		return false;
	}

	auto counter = item->counter;
	try
	{
		item->task();
	}
	catch (...)
	{
		Engine::GetLogger().LogError(LogCategory::Engine, "Main thread caught job exception");
	}
	counter->fetch_sub(1, std::memory_order_release);
	return true;
}

// SDL Event Processing (main thread only)
void JobSystem::ProcessEvents()
{
	Engine::GetInputManager().ResetWheelMotion();
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL3_ProcessEvent(&event);

		if (ImGui::IsAnyItemActive() == false)
		{
			Engine::GetInputManager().InputPollEvent(event);
		}

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			Engine::GetGameStateManager().SetGameState(State::UNLOAD);
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			Engine::Instance().ResetDeltaTime();
			if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::DX)
			{
				dynamic_cast<DXRenderManager*>(Engine::GetRenderManager())->SetResize();
			}
			SDL_FALLTHROUGH;
		case SDL_EVENT_WINDOW_MOVED:
		case SDL_EVENT_WINDOW_MINIMIZED:
		case SDL_EVENT_WINDOW_MAXIMIZED:
		case SDL_EVENT_WINDOW_RESTORED:
			Engine::Instance().ResetDeltaTime();
			break;
		default:
			break;
		}

		if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
		{
			Engine::GetWindow().UpdateWindowGL(event);
		}
	}
}
