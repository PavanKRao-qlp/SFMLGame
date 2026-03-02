#include "Diag/Assert.h"
#include "Diag/Logger.h"

namespace Umbra {
    void HandleAssertionFailure(const String& _expression, const String& _message, const String& _file, int _line) {
        Logger::GetInstance()->Log(ELogLevel::Critical, _file.c_str(), _line, "Assertion failed [%s] :: %s",
            _expression.c_str(), _message.c_str());

#ifdef DEBUG
        std::abort();
#else
        std::exit(EXIT_FAILURE);
#endif
    }
} // namespace Umbra
