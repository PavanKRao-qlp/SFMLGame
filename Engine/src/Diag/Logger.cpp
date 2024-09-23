#include "Diag/Logger.h"

Logger::~Logger()
{
}

Logger::Logger()
{
}

void Logger::LogToConsole(LogType _verbosity, std::string _message)
{
    printf(_message.c_str());
    printf("\n");
}

void Logger::Log(LogType _verbosity, std::string _message)
{
    Logger::getInstance()->LogToConsole(_verbosity, _message);
}
