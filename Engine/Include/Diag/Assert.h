#pragma once
#include "EnginePCH.h"

#define DEBUG 1

#define UM_S_ASSERT(expr) assert(expr)

#define UM_ASSERT(expr, msg)                                                   \
    do {                                                                       \
        if (!(expr))                                                           \
            Umbra::HandleAssertionFailure(#expr, msg, __FILE__, __LINE__);    \
    } while (0)


namespace Umbra {
    void HandleAssertionFailure(const String& _expression, const String& _message, const String& _file, int _line);
} // namespace Umbra
