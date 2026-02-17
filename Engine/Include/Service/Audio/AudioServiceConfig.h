#pragma once
#include "EnginePCH.h"

namespace Umbra {

    /// @brief Sound group categories for volume mixing
    enum class ESoundGroup : uint8 {
        Master,
        SFX,
        Music,
        Count
    };

    /// @brief Configuration for the audio service
    struct AudioServiceConfig {
        float MasterVolume        = 1.0f;
        float SFXVolume           = 1.0f;
        float MusicVolume         = 1.0f;
        uint32 InitialSoundCapacity = 64;
    };

} // namespace Umbra
