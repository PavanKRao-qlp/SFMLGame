#pragma once
#include "EnginePCH.h"
#include "Service/Audio/AudioHandle.h"
#include "Service/Audio/AudioServiceConfig.h"

#include <miniaudio.h>

namespace Umbra {

    /// @brief Internal sound data stored in slot array within AudioService
    struct SoundData {
        ma_sound Sound;
        uint32 Generation = 0;
        bool bActive      = false;
        ESoundGroup Group  = ESoundGroup::SFX;
        String FilePath;
        bool bLooping      = false;
        float Volume       = 1.0f; // Per-sound volume
    };

    /// @brief Audio service that manages sound playback via miniaudio
    /// Sounds are accessed via handles, ECS components hold handles and sync with service
    class AudioService {
    public:
        AudioService();
        explicit AudioService(const AudioServiceConfig& _config);
        ~AudioService();

        // ============== Playback ==============

        /// @brief Plays a sound from file, returns handle
        /// @param _filePath Path to audio file (WAV, MP3, FLAC, etc.)
        /// @param _group Sound group for volume mixing
        /// @param _looping Whether the sound should loop
        /// @return Handle to the created sound
        SoundHandle PlaySound(const String& _filePath, ESoundGroup _group, bool _looping = false);

        /// @brief Stops a sound and marks it for cleanup
        void StopSound(SoundHandle _handle);

        /// @brief Pauses a sound
        void PauseSound(SoundHandle _handle);

        /// @brief Resumes a paused sound
        void ResumeSound(SoundHandle _handle);

        /// @brief Checks if a sound is currently playing
        bool IsSoundPlaying(SoundHandle _handle);

        // ============== Volume ==============

        /// @brief Sets per-sound volume (multiplied with group and master)
        void SetSoundVolume(SoundHandle _handle, float _volume);

        /// @brief Sets volume for a sound group, reapplies to all active sounds in group
        void SetGroupVolume(ESoundGroup _group, float _volume);

        /// @brief Gets current volume for a sound group
        float GetGroupVolume(ESoundGroup _group) const;

        // ============== Control ==============

        /// @brief Stops all active sounds
        void StopAllSounds();

        /// @brief Stops all sounds in a specific group
        void StopGroup(ESoundGroup _group);

        /// @brief Cleans up finished one-shot sounds (reclaims slots)
        void Update();

        // ============== Validation ==============

        /// @brief Checks if a sound handle is still valid
        bool IsSoundValid(SoundHandle _handle) const;

    private:
        /// @brief Resolves a handle to internal data, returns nullptr if invalid
        SoundData* GetSoundData(SoundHandle _handle);
        const SoundData* GetSoundData(SoundHandle _handle) const;

        /// @brief Calculates effective volume for a sound
        float CalculateEffectiveVolume(const SoundData& _data) const;

        /// @brief Reapplies volume to a sound based on current group/master settings
        void ReapplyVolume(SoundData& _data);

    private:
        AudioServiceConfig mConfig;
        ma_engine mEngine;
        bool bEngineInitialized = false;

        // Sound storage (generational slot array + free list)
        Vector<SoundData> mSounds;
        Vector<uint32> mFreeIndices;

        // Group volumes: indexed by (int)ESoundGroup
        float mGroupVolumes[static_cast<int>(ESoundGroup::Count)];
    };

} // namespace Umbra
