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
    template <typename... Args>
    static void Log(LogType _verbosity, char *_message, Args... _args);
};

template <typename... Args>
static inline void Logger::Log(LogType _verbosity, char *_message, Args... _args)
{
    int size = std::snprintf(nullptr, 0, _message, _args...);
    if (size <= 0)
    {
        Logger::getInstance()->LogToConsole(LogType::Error, "LOG Formatting error!");
        return;
    }
    std::string formattedMessage(size, '\0');
    std::snprintf(&formattedMessage[0], size + 1, _message, _args...);
    Logger::getInstance()->LogToConsole(_verbosity, formattedMessage);
}