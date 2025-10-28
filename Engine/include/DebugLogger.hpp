#pragma once
#include <string>
#include <fstream> //file output
#include <chrono> //time
#include <mutex> 

enum class Severity
{
	Verbose,		// This option is for minor messages, which could be useful 
	Debug,			// This should only be used when you are currently trying to debug something
	Event,			// These are general events, such as key press, exit state, enter state, enter state finish
	Error,			// This is for an error, such as an asset is not found
};

enum class LogCategory
{
	Engine,
	Graphic,
	Level,
	Object,
	Physics,
	Sound,
	Game,
	Editor
};

class DebugLogger
{
public:
	DebugLogger(std::chrono::steady_clock::time_point startTime, Severity severity = Severity::Verbose);
	~DebugLogger();

	void LogError(LogCategory category, std::string text);
	void LogEvent(LogCategory category, std::string text);
	void LogDebug(LogCategory category, std::string text);
	void LogVerbose(LogCategory category, std::string text);

private:
	void Log(Severity severity, LogCategory category, std::string displayText);
	float GetSecondsSinceStart();

	const char* SeverityToString(Severity severity);
	const char* CategoryToString(LogCategory category);

	std::ofstream outStream;
	Severity minLevel;
	std::chrono::steady_clock::time_point startTime;
	
	std::mutex logMutex;
};