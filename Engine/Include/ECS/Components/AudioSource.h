#pragma once
#include "ECS/Component.h"
#include "Service/Audio/AudioHandle.h"
#include "Service/Audio/AudioServiceConfig.h"

namespace Umbra {

    /// @brief ECS Component that bridges an entity to the AudioService
    /// Holds a handle into the audio service and playback configuration
    struct AudioSourceComponent : public Component {
    public:
        // Handle into AudioService
        SoundHandle Handle;

        // Configuration
        String FilePath;                        // Audio file path
        ESoundGroup Group = ESoundGroup::SFX;   // Sound group for volume mixing
        float Volume      = 1.0f;               // Per-source volume
        bool bLooping     = false;
        bool bAutoPlay    = true;               // Play on creation

        // Sync flags
        bool bNeedsSoundCreation = true;        // Deferred creation flag
        bool bSyncEnabled        = true;

        inline bool HasValidSound() const {
            return Handle.IsValid() && !bNeedsSoundCreation;
        }
    };

} // namespace Umbra
