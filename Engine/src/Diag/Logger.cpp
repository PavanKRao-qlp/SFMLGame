#include "Diag/Logger.h"


namespace Umbra {
    void Logger::Initialize(const Config& _loggerConfig) {
        Logger::GetInstance()->mConfig = _loggerConfig;
    }

    void Logger::LogToConsole(ELogLevel _verbosity, const char* _message) {
        std::clog << LogLevelColor(_verbosity) << _message << "\033[0m" << std::endl;
    }

    String Logger::LogLevelString(ELogLevel _verbosity) {
        switch (_verbosity) {
        case ELogLevel::Trace:
            return "[TRACE] : ";
        case ELogLevel::Debug:
            return "[DEBUG] : ";
        case ELogLevel::Info:
            return "[INFO] : ";
        case ELogLevel::Warning:
            return "[WARNING] : ";
        case ELogLevel::Error:
            return "[ERROR] : ";
        case ELogLevel::Critical:
            return "[CRITICAL] : ";
        default:
            return "[UNKNOWN] : ";
        }
    }

    Logger::~Logger() {
        std::clog << "LoggerDone\n";
    }

} // namespace Umbra
