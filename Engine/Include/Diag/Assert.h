#pragma once
#include "Diag/Logger.h"
#include "EnginePCH.h"

#define DEBUG = 1;
#define UM_ASSERT(expr, msg) \
    if (!(expr))             \
    Umbra::HandleAssertionFailure(#expr, msg, __FILE__, __LINE__)


namespace Umbra {
    inline void HandleAssertionFailure(
        const std::string& _expression, std::string _message, const std::string& _file, int _line) {
        Logger::Log(LogType::Assert, "Assertion failed [%s] :: %s \n in : %s at  %d", _expression.c_str(),
            _message.c_str(), _file.c_str(), _line);

#ifdef DEBUG
        std::abort();
#else
        std::exit(EXIT_FAILURE);
#endif
    }
} // namespace Umbra
