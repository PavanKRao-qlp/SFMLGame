#pragma once
#include "Core/Event.h"
#include "Core/Singleton.h"
#include "Diag/Assert.h"
#include "Diag/Logger.h"
#include "EnginePCH.h"
#include "Math/Vector.h"

namespace Umbra {
    struct FGameConfig {
        Math::Vector2i WindowSize = Math::Vector2i(800, 800);
        bool bFullScreen;
        float FixedDeltaTime      = 0.02f;
        float MaxPhysicsDeltaTime = 0.1f;
    };

    struct GEngineStatics {
        class AppWindow* AppWindowPtr;
        struct FGameConfig* GameConfig;
    } inline GEngineStatics;

} // namespace Umbra
