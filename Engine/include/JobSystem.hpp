//Author: DOYEONG LEE
//Project: CubeEngine
//File: JobSystem.hpp
#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <functional>
#include <atomic>
#include <vector>
#include <optional>
#include <memory>
#include <SDL3/SDL.h>

// Represents a single unit of work to be executed by a worker thread
struct Job
{
	std::function<void()> task;
	std::shared_ptr<std::atomic<int>> counter; // Shared completion counter
};

// Handle returned by QueueWork / QueueParallelWork to track job completion
class JobHandle
{
public:
	JobHandle() = default;

	bool IsComplete() const
	{
		return !counter || counter->load(std::memory_order_acquire) <= 0;
	}

private:
	std::shared_ptr<std::atomic<int>> counter;
	friend class JobSystem;
};

// Thread-safe work queue for distributing jobs to worker threads
class JobQueue
{
public:
	void Push(Job job);
	std::optional<Job> TryPop();
	std::optional<Job> WaitAndPop(std::atomic<bool>& running);
	void NotifyAll();

private:
	std::deque<Job> jobs;
	std::mutex queueMutex;
	std::condition_variable cv;
};

// Multi-threaded job system that replaces the old ThreadManager
class JobSystem
{
public:
	JobSystem() : running(false) {}
	~JobSystem() { Stop(); }

	// Start worker thread pool (0 = auto: hardware_concurrency - 1)
	void Start(uint32_t threadCount = 0);
	void Stop();

	// Submit a single job, returns a handle for synchronization
	JobHandle QueueWork(std::function<void()> task);

	// Submit a parallel-for job that splits 'count' items into batches
	JobHandle QueueParallelWork(
		uint32_t count,
		std::function<void(uint32_t begin, uint32_t end)> body,
		uint32_t batchSize = 64
	);

	// Block until the specified job is complete (main thread helps execute jobs)
	void WaitForWork(const JobHandle& handle);

	// Block until all specified jobs are complete
	void WaitForAll(const std::vector<JobHandle>& handles);

	// Process SDL events on the main thread
	void ProcessEvents();

	uint32_t GetWorkerCount() const
	{
		return static_cast<uint32_t>(workerThreads.size());
	}

private:
	// Main loop for each worker thread
	void WorkerLoop(uint32_t workerIndex);

	// Try to pop and execute one job from the queue (used by main thread during WaitForWork)
	bool ExecuteOneJob();

	std::vector<std::thread> workerThreads;
	JobQueue jobQueue;
	std::atomic<bool> running;
};
