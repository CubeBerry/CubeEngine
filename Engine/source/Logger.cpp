//Author: DOYEONG LEE
//Project: CubeEngine
//File: Logger.cpp
#include <iostream> // cout.rdbuf
#include <sstream>   // FileName
#include <iomanip>   // Time/Date
#include "Logger.hpp"

Logger::Logger(std::chrono::steady_clock::time_point startTime, Severity severity)
	: minLevel(severity), startTime(startTime)
{
	char choice = 'n';
	std::cout << "Save Logs History? (y/n): ";
	std::cin >> choice;

	if (choice == 'y' || choice == 'Y')
	{
		auto const timeForFilename = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(timeForFilename);
		std::tm localTm;

#ifdef _WIN32
		localtime_s(&localTm, &t);
#else
		localtime_r(&t, &localTm);
#endif

		std::stringstream ss;
		ss << "Log_" << std::put_time(&localTm, "%Y-%m-%d_%H-%M-%S") << ".csv";
		std::string filename = ss.str();

		outStream.open(filename);

		// Add a header (column titles) to the first line of the .csv file
		if (outStream.is_open())
		{
			outStream << "Timestamp,Severity,Category,Message\n";
			outStream.flush();
		}

		std::cout << "Saving logs to '" << filename << "'" << std::endl;
	}
	else
	{
		outStream.set_rdbuf(std::cout.rdbuf());
	}
}

Logger::~Logger()
{
	outStream.flush();
	outStream.close();
}

void Logger::LogError(LogCategory category, std::string text)
{
	Log(Severity::Error, category, text);
}

void Logger::LogEvent(LogCategory category, std::string text)
{
	Log(Severity::Event, category, text);
}


void Logger::LogDebug(LogCategory category, std::string text)
{
	Log(Severity::Debug, category, text);
}

void Logger::LogVerbose(LogCategory category, std::string text)
{
	Log(Severity::Verbose, category, text);
}

void Logger::Log(Severity severity, LogCategory category, std::string displayText)
{
	//std::lock_guard<std::mutex> lock(logMutex);

	if (severity < minLevel)
	{
		return;
	}

	// Get current time information
	auto const now = std::chrono::system_clock::now();
	auto const now_t = std::chrono::system_clock::to_time_t(now);
	auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::tm localTm;
#ifdef _WIN32
	localtime_s(&localTm, &now_t);
#else
	localtime_r(&now_t, &localTm);
#endif

	std::stringstream ss_timestamp;
	ss_timestamp << std::put_time(&localTm, "%H:%M:%S");
	ss_timestamp << '.' << std::setw(3) << std::setfill('0') << ms.count();
	std::string timestampStr = ss_timestamp.str();

	std::stringstream ss_console;
	ss_console << '[' << timestampStr << "]\t";
	ss_console << '[' << SeverityToString(severity) << "]\t";
	ss_console << '[' << CategoryToString(category) << "]\t";
	ss_console << displayText << '\n';

	std::string consoleMessage = ss_console.str();
	std::cout << consoleMessage;
	std::cout.flush();

	if (outStream.rdbuf() != std::cout.rdbuf() && outStream.is_open())
	{
		// Escape double quotes (") with two double quotes ("") for CSV standard
		std::string escapedMessage = displayText;
		size_t pos = escapedMessage.find('"');
		while (pos != std::string::npos)
		{
			escapedMessage.replace(pos, 1, "\"\"");
			pos = escapedMessage.find('"', pos + 2);
		}

		// Create CSV format string (comma-separated)
		std::stringstream ss_csv;
		ss_csv << timestampStr << ",";
		ss_csv << SeverityToString(severity) << ",";
		ss_csv << CategoryToString(category) << ",";
		// Enclose the entire message in double quotes in case it contains commas
		ss_csv << '"' << escapedMessage << '"' << '\n';

		outStream << ss_csv.str();
		outStream.flush();
	}
}

float Logger::GetSecondsSinceStart()
{
	using clock = std::chrono::steady_clock;
	using second = std::chrono::duration<float>;

	return std::chrono::duration_cast<second>(clock::now() - startTime).count();
}

const char* Logger::SeverityToString(Severity severity)
{
	switch (severity)
	{
	case Severity::Verbose:
		return "Verb";
	case Severity::Debug:
		return "Debug";
	case Severity::Event:
		return "Event";
	case Severity::Error:
		return "Error";
	default:
		return "NONE";
	}
}

const char* Logger::CategoryToString(LogCategory category)
{
	switch (category)
	{
	case LogCategory::Engine:
		return "Engine";
	case LogCategory::Graphic:
		return "Graphic";
	case LogCategory::Level:
		return "Level";
	case LogCategory::Object:
		return "Object";
	case LogCategory::Physics:
		return "Physics";
	case LogCategory::Sound:
		return "Sound";
	case LogCategory::Camera:
		return "Camera";
	case LogCategory::Game:
		return "Game";
	case LogCategory::Editor:
		return "Editor";
	default:
		return "Unknown";
	}
}