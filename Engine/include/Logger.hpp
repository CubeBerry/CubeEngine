//Author: DOYEONG LEE
//Project: CubeEngine
//File: Logger.hpp
#pragma once
#include <string>
#include <fstream> //file output
#include <chrono> //time
#include <mutex> 

enum class Severity
{
	Verbose,		// Minor Messages: Useful secondary info, often for minor debugging.
	Debug,			// Debugging Only: Detailed logs specifically needed during active debugging.
	Event,			// General Events: Standard occurrences like key presses or state changes.
	Error,			// Error: Critical issues, such as an asset failing to load (asset not found).
};

enum class LogCategory
{
	Engine,
	Graphics,
	Level,
	Object,
	Physics,
	Sound,
	Camera,
	Game,
	Editor
};

class Logger
{
public:
	Logger(std::chrono::steady_clock::time_point startTime, Severity severity = Severity::Verbose);
	~Logger();

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