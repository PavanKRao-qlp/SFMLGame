#pragma once
#include "Core/Event.h"
#include "Core/Singleton.h"
#include "Diag/Assert.h"
#include "Diag/Logger.h"
#include "EnginePCH.h"

namespace Umbra {
    struct GEngineStatics {
        class AppWindow* AppWindowPtr;
    } inline GEngineStatics;

} // namespace Umbra
