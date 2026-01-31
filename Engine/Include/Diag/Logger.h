#pragma once
#include "Assert.h"
#include "Core/Singleton.h"
#include "EnginePCH.h"

namespace Umbra {

    enum class ELogLevel : uint8 {
        Trace    = 1, // Detailed debug information
        Debug    = 2, // Debugging messages
        Info     = 3, // General information
        Warning  = 4, // Potentially problematic situations
        Error    = 5, // Recoverable errors
        Critical = 6 // Unrecoverable critical errors
    };

    class Logger : public Singleton<Logger> {
    public:
        struct Config {
            ELogLevel ConsoleLogLevel = ELogLevel::Trace;
            ELogLevel HUDLogLevel     = ELogLevel::Debug;
            ELogLevel FileLogLevel    = ELogLevel::Warning;
            // - Path to write logFile
            Umbra::String LogPath = "";
            // - Should LogLevel::Info and below should show file and line
            bool bVerboseLog  = false;
            bool bEnableColor = true;
            bool bEnable      = true;
        };
        static void Initialize(const Config& _loggerConfig);

        template <typename... Args>
        void Log(
            ELogLevel _verbosity, const char* _traceFile, const int _traceLineNo, const char* _message, Args... _args);

    private:
        void LogToConsole(ELogLevel _verbosity, const char* _message);
        String LogLevelString(ELogLevel _verbosity);

        constexpr const char* LogLevelColor(ELogLevel _verbosity) {
            switch (_verbosity) {
            case ELogLevel::Trace:
                return "\033[90m"; // Bright Black
            case ELogLevel::Debug:
                return "\033[36m"; // Cyan
            case ELogLevel::Info:
                return "\033[0m"; // Reset
            case ELogLevel::Warning:
                return "\033[33m"; // Yellow
            case ELogLevel::Error:
                return "\033[31m"; // Red
            case ELogLevel::Critical:
                return "\033[41m\033[37m"; // White on Red
            default:
                return "\033[0m";
            }
        }

        template <typename... Args>
        String FormatString(const char* _message, Args... _args);

        Config mConfig;

        friend class Singleton<Logger>;
        Logger() = default;
        ~Logger();
    };

    template <typename... Args>
    void Logger::Log(
        ELogLevel _verbosity, const char* _traceFile, const int _traceLineNo, const char* _message, Args... _args) {
        if (!mConfig.bEnable) {
            return;
        }
        String FormattedMessage = LogLevelString(_verbosity) + FormatString(_message, _args...);
        if (mConfig.bVerboseLog || _verbosity >= ELogLevel::Warning) {
            FormattedMessage += FormatString(" - at %s(%d)", _traceFile, _traceLineNo);
        }
        if (mConfig.ConsoleLogLevel <= _verbosity) {
            LogToConsole(_verbosity, FormattedMessage.c_str());
        }
    }

    template <typename... Args>
    String Logger::FormatString(const char* _message, Args... _args) {
        int size = std::snprintf(nullptr, 0, _message, _args...);
        UM_S_ASSERT(size > 0);
        String formattedMessage(size, '\0');
        std::snprintf(&formattedMessage[0], size + 1, _message, _args...);
        return formattedMessage;
    }

} // namespace Umbra

#define UMBRA_LOG_TRACE(message, ...) \
    Umbra::Logger::GetInstance()->Log(Umbra::ELogLevel::Trace, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define UMBRA_LOG_DEBUG(message, ...) \
    Umbra::Logger::GetInstance()->Log(Umbra::ELogLevel::Debug, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define UMBRA_LOG_INFO(message, ...) \
    Umbra::Logger::GetInstance()->Log(Umbra::ELogLevel::Info, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define UMBRA_LOG_WARNING(message, ...) \
    Umbra::Logger::GetInstance()->Log(Umbra::ELogLevel::Warning, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define UMBRA_LOG_ERROR(message, ...) \
    Umbra::Logger::GetInstance()->Log(Umbra::ELogLevel::Error, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define UMBRA_LOG_CRITICAL(message, ...) \
    Umbra::Logger::GetInstance()->Log(   \
        Umbra::ELogLevel::Critical, __FILE__, __LINE__, message, ##__VA_ARGS__)
