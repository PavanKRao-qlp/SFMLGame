#pragma once
#include "EnginePCH.h"
#include "Core/Singleton.h"

enum class LogType
{	
	Verbose,
    Trace,
	Warning,
	Error,
	FatalError
};

class Logger : protected Singleton<Logger>
{
private:
    /* data */
public:
    Logger(/* args */);
    ~Logger();
    void LogToConsole(LogType _verbosity, std::string _message);
    static void Log(LogType _verbosity, std::string _message);
};
