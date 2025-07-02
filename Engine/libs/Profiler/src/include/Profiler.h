// (C) 2025 DigiPen (USA) Corporation

#pragma once
#include <deque>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <vector>


#define DLLEXPORT __declspec(dllexport)


extern "C" void DLLEXPORT SyncEngineProfilerEnterFunc(void* address);
extern "C" void DLLEXPORT SyncEngineProfilerExitFunc(void* address);


namespace sync_engine
{

	class Profiler
	{
		/*
		 * nullptr indicates an exit of a function.
		 *
		 * For example, given the following code:
		 *     void f()
		 *     {
		 *         g();
		 *	       h();
		 *     }
		 *     void g()
		 *     {
		 *         i();
		 *     }
		 *     void h() {}
		 *	   void i() {}
		 *
		 *	   f();
		 *
		 * The flattened call stack will look like this:
		 *     f  g  i  nullptr  nullptr  h  nullptr  nullptr
		 *	               i        g           h        f
		 */
		struct FunctionCall
		{
			void* address;
			unsigned long long tsc;

			constexpr bool operator==(const FunctionCall& other) const noexcept = default;
		};
		using FlattenedCallStack = std::vector<FunctionCall>;


	public:
		DLLEXPORT static /*thread_local*/ Profiler& GetInstance();

		DLLEXPORT static void Initialize(std::filesystem::path outputDir);
		DLLEXPORT static void Shutdown();

		DLLEXPORT static void StartProfiling();
		DLLEXPORT static void StopProfiling();

		DLLEXPORT static void SetCustomMarkOnly(bool isCustomMarkOnly);

		DLLEXPORT void MarkFrame();

		DLLEXPORT void BeginCustomMark(unsigned int index);
		DLLEXPORT void EndCustomMark();

		explicit Profiler();
		Profiler(const Profiler& other) = delete;
		Profiler(Profiler&& other) noexcept = default;
		Profiler& operator=(const Profiler& other) = delete;
		Profiler& operator=(Profiler&& other) noexcept = default;
		~Profiler();

		void EnterFunc(void* address, unsigned long long tsc);
		void ExitFunc(unsigned long long tsc);

	private:
		void closeOutputFile();

	private:
		static void initializePlatformDependent();
		static void shutdownPlatformDependent();
		static void createReport();


	public:
		inline static bool mIsInitialized = false;

	private:
		// Trade-off: Not properly synced between threads, speed is prioritized since it's being checked every function call.
		inline static bool mIsProfiling = false;
		inline static bool mIsCustomMarkOnly = false;

		inline static std::filesystem::path mOutputDir;
		inline static std::atomic<int> mNextThreadID = 1;
		inline static std::vector<unsigned long long> mHaltedTSCs;

		inline static std::deque<Profiler*> mInstances;
		inline static std::mutex mInstancesMutex;

		FlattenedCallStack mCurrentCallStack;
		int mThreadID = 0;
		std::ofstream mOutputFile;


		static inline std::filesystem::path INTERMEDIATE_FILE_EXTENSION = ".seprf";
		static inline std::filesystem::path REPORT_FILE_EXTENSION = ".txt";
		static constexpr FunctionCall FRAME_MARKER{ nullptr, 0xFFFFFFFFFFFFFFFFULL };

		static constexpr unsigned int CUSTOM_MARKER_MAX_COUNT = 100;
		static constexpr size_t CUSTOM_MARKER_ADDRESS_MAX = static_cast<size_t>(-1) - 1;
		static constexpr size_t CUSTOM_MARKER_ADDRESS_MIN = CUSTOM_MARKER_ADDRESS_MAX - CUSTOM_MARKER_MAX_COUNT - 1;
	};

}


#undef DLLEXPORT
