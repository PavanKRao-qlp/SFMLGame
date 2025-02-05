#include "Diag/Logger.h"


namespace Umbra {
    void Logger::Initialize(const Config& _loggerConfig) {
        Logger::GetInstance()->mConfig = _loggerConfig;
    }

    void Logger::LogToConsole(ELogLevel _verbosity, const char* _message) {
        std::clog << LogLevelColor(_verbosity) << _message << "\033[0m" << std::endl;
    }

    String Logger::LogLevelString(ELogLevel __verbosity) {
        switch (__verbosity) {
        case ELogLevel::Trace:
            return "[TRACE] : ";
        case ELogLevel::Debug:
            return "[DEBUG] : ";
        case ELogLevel::Info:
            return "[INFO] : ";
        case ELogLevel::Warning:
            return "[WARNING] : ";
        case ELogLevel::Error:
            return "[ERROR] :";
        case ELogLevel::Critical:
            return "[CRITICAL] : ";
        default:
            return "[UNKNOWN] : ";
        }
    }
    //     printf("\n");
    constexpr const char* Logger::LogLevelColor(ELogLevel __verbosity) {
        switch (__verbosity) {
        case ELogLevel::Trace:
            return "\033[90m";
            break; // Bright Black
        case ELogLevel::Debug:
            return "\033[36m";
            break; // Cyan
        case ELogLevel::Info:
            return "\033[0m";
            break; // Reset
        case ELogLevel::Warning:
            return "\033[33m";
            break; // Yellow
        case ELogLevel::Error:
            return "\033[31m";
            break; // Red
        case ELogLevel::Critical:
            return "\033[41m\033[37m";
            break; // White on Red
        default:
            return "\033[0m";
        }
        return nullptr;
    }

    Logger::~Logger() {
        std::clog << "LoggerDone\n";
    }

} // namespace Umbra
