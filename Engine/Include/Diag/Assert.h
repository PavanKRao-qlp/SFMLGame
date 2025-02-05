#pragma once
#include "Diag/Logger.h"
#include "EnginePCH.h"

#define DEBUG = 1;

#define UM_S_ASSERT(expr) assert(expr);

#define UM_ASSERT(expr, msg) \
    if (!(expr))             \
    Umbra::HandleAssertionFailure(#expr, msg, __FILE__, __LINE__)


namespace Umbra {
    inline void HandleAssertionFailure(const String& _expression, String _message, const String& _file, int _line) {
        // Logger::GetInstance()->Log(ELogLevel::Critical, _file.c_str(), _line, "Assertion failed [%s] :: %s",
        //     _expression.c_str(), _message.c_str());

#ifdef DEBUG
        std::abort();
#else
        std::exit(EXIT_FAILURE);
#endif
    }
} // namespace Umbra
