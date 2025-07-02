// (C) 2025 DigiPen (USA) Corporation

#include "Profiler.h"

#include <stack>

#include "Internal.h"


void SyncEngineProfilerEnterFunc(void* address)
{
    if (!sync_engine::Profiler::mIsInitialized)
    {
        return;
    }

    sync_engine::Profiler::GetInstance().EnterFunc(address, sync_engine::get_tsc_enter());
}


void SyncEngineProfilerExitFunc([[maybe_unused]] void* address)
{
    if (!sync_engine::Profiler::mIsInitialized)
    {
        return;
    }

    sync_engine::Profiler::GetInstance().ExitFunc(sync_engine::get_tsc_exit());
}


namespace sync_engine
{


    Profiler& Profiler::GetInstance()
    {
        thread_local Profiler instance;
        return instance;
    }


    void Profiler::Initialize(std::filesystem::path outputDir)
    {
        using namespace std::filesystem;

        mOutputDir = std::move(outputDir);
        if (exists(mOutputDir))
        {
            remove_all(mOutputDir);
        }
        create_directories(mOutputDir);

        mIsInitialized = true;
    }


    void Profiler::Shutdown()
    {
        {
            std::lock_guard lock{ mInstancesMutex };
            mIsInitialized = false;

            for (Profiler* instance : mInstances)
            {
                instance->MarkFrame();
                instance->closeOutputFile();
            }
        }

        createReport();

        shutdownPlatformDependent();
    }


    void Profiler::StartProfiling()
    {
        if (mIsProfiling)
        {
            return;
        }

        mIsProfiling = true;
        get_tsc_exit();  // To make sure mIsProfiling = true is set before any other operations are performed.
    }


    void Profiler::StopProfiling()
    {
        if (!mIsProfiling)
        {
            return;
        }

        mIsProfiling = false;
        mHaltedTSCs.emplace_back(get_tsc_exit());
    }


    void Profiler::SetCustomMarkOnly(bool isCustomMarkOnly)
    {
        mIsCustomMarkOnly = isCustomMarkOnly;
    }


    void Profiler::MarkFrame()
    {
        if (mCurrentCallStack.empty())
        {
            return;
        }

        if (!mOutputFile.is_open())
        {
            mOutputFile.open((mOutputDir / std::to_string(mThreadID)).replace_extension(INTERMEDIATE_FILE_EXTENSION), std::ios_base::out | std::ios_base::binary);
        }
        mOutputFile.write(reinterpret_cast<const char*>(mCurrentCallStack.data()), static_cast<std::streamsize>(mCurrentCallStack.size() * sizeof(FlattenedCallStack::value_type)));
        mOutputFile.write(reinterpret_cast<const char*>(&FRAME_MARKER), sizeof(FRAME_MARKER));
        mCurrentCallStack.clear();
    }


    void Profiler::BeginCustomMark(unsigned int index)
    {
        if (index >= CUSTOM_MARKER_MAX_COUNT)
        {
            throw std::runtime_error{ "Exceeded the maximum number of custom markers." };
        }

        // For now, let's just treat it as a function call.
        mCurrentCallStack.emplace_back(reinterpret_cast<void*>(CUSTOM_MARKER_ADDRESS_MIN + index), get_tsc_enter());
    }


    void Profiler::EndCustomMark()
    {
        mCurrentCallStack.emplace_back(nullptr, get_tsc_exit());
    }


    Profiler::Profiler()
        // For some reason, opening a file here causes a crash in Release mode.
        //: mOutputFile((mIntermediateDir / std::to_string(mNextThreadID++)).replace_extension(INTERMEDIATE_FILE_EXTENSION), std::ios_base::out | std::ios_base::binary)
        : mThreadID(mNextThreadID++)
    {
        mCurrentCallStack.reserve(4096);

        std::lock_guard lock{ mInstancesMutex };
        mInstances.emplace_back(this);
    }


    Profiler::~Profiler()
    {
        std::lock_guard lock{ mInstancesMutex };
        if (!mIsInitialized)
        {
            // Already taken care by Shutdown()
            return;
        }

        MarkFrame();
        closeOutputFile();
        mInstances.erase(std::ranges::find(mInstances, this));
    }


    void Profiler::EnterFunc(void* address, unsigned long long tsc)
    {
        if (!mIsProfiling || mIsCustomMarkOnly)
        {
            return;
        }

        mCurrentCallStack.emplace_back(address, tsc);
    }


    void Profiler::ExitFunc(unsigned long long tsc)
    {
        if (!mIsProfiling || mIsCustomMarkOnly)
        {
            return;
        }

        mCurrentCallStack.emplace_back(nullptr, tsc);
    }


    void Profiler::closeOutputFile()
    {
        mOutputFile.close();
    }


    void Profiler::createReport()
    {
        using namespace std::filesystem;

        struct CallStackEntry
        {
            unsigned long long startTSC = 0;
            unsigned long long childrenDuration = 0;
            std::streamoff writePos = 0;
        };

        initializePlatformDependent();

        std::unordered_map<void*, std::string> functionNames;

        constexpr size_t blockSize = 2 * 1024 * 1024;
        FunctionCall* block = new FunctionCall[blockSize / sizeof(FunctionCall)];

        constexpr int INDENT = 4;
        constexpr const char* TEMP_TSC_COUNT = "18446744073709551615 | 18446744073709551615";  // std::numeric_limits<unsigned long long>::max()
        constexpr int TSC_COUNT_LENGTH = 20;

        directory_iterator dirIt{ mOutputDir };
        for (const directory_entry& threadEntry : dirIt)
        {
            if (!threadEntry.is_regular_file() || threadEntry.path().extension() != INTERMEDIATE_FILE_EXTENSION)
            {
                continue;
            }

            std::ifstream record{ threadEntry.path(), std::ios_base::in | std::ios_base::binary };

            path reportPath = threadEntry.path();
            reportPath.replace_extension(REPORT_FILE_EXTENSION);
            std::ofstream report{ reportPath };
            report << "# Frame 0\n" << std::setfill('.') << std::right;

            size_t frameCount = 0;
            size_t nextHaltedTSCIndex = 0;
            unsigned long long nextHaltedTSC = mHaltedTSCs.empty() ? static_cast<unsigned long long>(-1) : mHaltedTSCs.front();
            std::stack<CallStackEntry> callStack;

            const auto lambdaProcessFunctionExit =
                [&callStack, &report](unsigned long long exitTsc)
                {
                    const auto& [startTSC, childrenDuration, writePos] = callStack.top();
                    const unsigned long long duration = exitTsc - startTSC;

                    const std::streampos end = report.tellp();
                    report.seekp(writePos);
                    report << std::setw(TSC_COUNT_LENGTH) << duration << " | ";
                    report << std::setw(TSC_COUNT_LENGTH) << duration - childrenDuration;
                    report.seekp(end);

                    callStack.pop();
                    if (!callStack.empty())
                    {
                        callStack.top().childrenDuration += duration;
                    }
                };

            while (!record.eof())
            {
                record.read(reinterpret_cast<char*>(block), sizeof(FunctionCall));
                const int functionCallsRead = static_cast<int>(record.gcount() / sizeof(FunctionCall));
                for (int i = 0; i < functionCallsRead; i++)
                {
                    const FunctionCall& call = block[i];
                    if (call == FRAME_MARKER)
                    {
                        report << "\n\n# Frame " << ++frameCount << '\n';
                        continue;
                    }

                    if (call.address == nullptr)
                    {
                        if (!callStack.empty())
                        {
                            lambdaProcessFunctionExit(call.tsc);
                        }
                        continue;
                    }

                    if (call.tsc >= nextHaltedTSC)
                    {
                        report << "\n----------Halted----------\n\n";
                        if (++nextHaltedTSCIndex < mHaltedTSCs.size())
                        {
                            nextHaltedTSC = mHaltedTSCs[nextHaltedTSCIndex];
                        }
                        else
                        {
                            nextHaltedTSC = static_cast<unsigned long long>(-1);
                        }

                        while (!callStack.empty())
                        {
                            lambdaProcessFunctionExit(nextHaltedTSC);
                        }
                    }

                    const std::string* functionName;
                    if (const auto it = functionNames.find(call.address);
                        it == functionNames.end())
                    {
                        if (const auto address = reinterpret_cast<size_t>(call.address);
                            CUSTOM_MARKER_ADDRESS_MIN <= address && address <= CUSTOM_MARKER_ADDRESS_MAX)
                        {
                            const auto newIt = functionNames.insert({ call.address, std::format("##CUSTOM MARK {}##", std::to_string(address - CUSTOM_MARKER_ADDRESS_MIN)) });
                            functionName = &newIt.first->second;
                        }
                        else
                        {
                            const auto newIt = functionNames.insert({ call.address, get_function_name(call.address) });
                            functionName = &newIt.first->second;
                        }
                    }
                    else
                    {
                        functionName = &it->second;
                    }

                    report << std::string(callStack.size() * INDENT, ' ');

                    CallStackEntry& entry = callStack.emplace();
                    entry.startTSC = call.tsc;

                    report << *functionName << " | ";
                    entry.writePos = report.tellp();
                    report << TEMP_TSC_COUNT << '\n';
                }
            }

            while (!callStack.empty())
            {
                lambdaProcessFunctionExit(nextHaltedTSC);
            }
        }
    }


}
