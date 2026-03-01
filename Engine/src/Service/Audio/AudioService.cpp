#include "Service/Audio/AudioService.h"

#include "Diag/Logger.h"
#include "Diag/MemoryTracker.h"

namespace Umbra {

    AudioService::AudioService() : AudioService(AudioServiceConfig{}) {}

    AudioService::AudioService(const AudioServiceConfig& _config) : mConfig(_config) {
        UMBRA_ALLOC_SCOPE(EMemoryCategory::Audio);
        mSounds.reserve(_config.InitialSoundCapacity);

        // Initialize group volumes from config
        mGroupVolumes[static_cast<int>(ESoundGroup::Master)] = _config.MasterVolume;
        mGroupVolumes[static_cast<int>(ESoundGroup::SFX)]    = _config.SFXVolume;
        mGroupVolumes[static_cast<int>(ESoundGroup::Music)]  = _config.MusicVolume;

        // Initialize miniaudio engine
        ma_engine_config engineConfig = ma_engine_config_init();
        ma_result result              = ma_engine_init(&engineConfig, &mEngine);
        if (result != MA_SUCCESS) {
            UMBRA_LOG_ERROR("Failed to initialize audio engine (error: {})", static_cast<int>(result));
            bEngineInitialized = false;
            return;
        }
        bEngineInitialized = true;
        UMBRA_LOG_INFO("AudioService initialized");
    }

    AudioService::~AudioService() {
        if (bEngineInitialized) {
            // Uninit all active sounds
            for (auto& sound : mSounds) {
                if (sound.bActive) {
                    ma_sound_uninit(&sound.Sound);
                    sound.bActive = false;
                }
            }
            ma_engine_uninit(&mEngine);
            UMBRA_LOG_INFO("AudioService destroyed");
        }
    }

    // ============== Playback ==============

    SoundHandle AudioService::PlaySound(const String& _filePath, ESoundGroup _group, bool _looping) {
        UMBRA_ALLOC_SCOPE(EMemoryCategory::Audio);
        if (!bEngineInitialized) {
            return SoundHandle::Invalid();
        }

        uint32 index;
        uint32 generation;

        if (!mFreeIndices.empty()) {
            // Reuse a free slot
            index = mFreeIndices.back();
            mFreeIndices.pop_back();
            generation = mSounds[index].Generation + 1;
        } else {
            // Allocate new slot
            index      = static_cast<uint32>(mSounds.size());
            generation = 0;
            mSounds.emplace_back();
        }

        SoundData& data = mSounds[index];
        data.Generation  = generation;
        data.bActive     = true;
        data.Group       = _group;
        data.FilePath    = _filePath;
        data.bLooping    = _looping;
        data.Volume      = 1.0f;

        // Initialize and start the sound
        ma_result result = ma_sound_init_from_file(
            &mEngine, _filePath.c_str(), 0, nullptr, nullptr, &data.Sound);
        if (result != MA_SUCCESS) {
            UMBRA_LOG_ERROR("Failed to load sound '{}' (error: {})", _filePath, static_cast<int>(result));
            data.bActive = false;
            mFreeIndices.push_back(index);
            return SoundHandle::Invalid();
        }

        ma_sound_set_looping(&data.Sound, _looping ? MA_TRUE : MA_FALSE);
        ReapplyVolume(data);
        ma_sound_start(&data.Sound);

        SoundHandle handle;
        handle.Index      = index;
        handle.Generation = generation;
        return handle;
    }

    void AudioService::StopSound(SoundHandle _handle) {
        SoundData* data = GetSoundData(_handle);
        if (data == nullptr) {
            return;
        }

        ma_sound_stop(&data->Sound);
        ma_sound_uninit(&data->Sound);
        data->bActive = false;
        mFreeIndices.push_back(_handle.Index);
    }

    void AudioService::PauseSound(SoundHandle _handle) {
        SoundData* data = GetSoundData(_handle);
        if (data == nullptr) {
            return;
        }
        ma_sound_stop(&data->Sound);
    }

    void AudioService::ResumeSound(SoundHandle _handle) {
        SoundData* data = GetSoundData(_handle);
        if (data == nullptr) {
            return;
        }
        ma_sound_start(&data->Sound);
    }

    bool AudioService::IsSoundPlaying(SoundHandle _handle) {
        SoundData* data = GetSoundData(_handle);
        if (data == nullptr) {
            return false;
        }
        return ma_sound_is_playing(&data->Sound) == MA_TRUE;
    }

    // ============== Volume ==============

    void AudioService::SetSoundVolume(SoundHandle _handle, float _volume) {
        SoundData* data = GetSoundData(_handle);
        if (data == nullptr) {
            return;
        }
        data->Volume = _volume;
        ReapplyVolume(*data);
    }

    void AudioService::SetGroupVolume(ESoundGroup _group, float _volume) {
        mGroupVolumes[static_cast<int>(_group)] = _volume;

        // Reapply volume to all active sounds in this group (or all if Master changed)
        for (auto& sound : mSounds) {
            if (!sound.bActive) {
                continue;
            }
            if (_group == ESoundGroup::Master || sound.Group == _group) {
                ReapplyVolume(sound);
            }
        }
    }

    float AudioService::GetGroupVolume(ESoundGroup _group) const {
        return mGroupVolumes[static_cast<int>(_group)];
    }

    // ============== Control ==============

    void AudioService::StopAllSounds() {
        for (uint32 i = 0; i < mSounds.size(); ++i) {
            if (mSounds[i].bActive) {
                ma_sound_stop(&mSounds[i].Sound);
                ma_sound_uninit(&mSounds[i].Sound);
                mSounds[i].bActive = false;
                mFreeIndices.push_back(i);
            }
        }
    }

    void AudioService::StopGroup(ESoundGroup _group) {
        for (uint32 i = 0; i < mSounds.size(); ++i) {
            if (mSounds[i].bActive && mSounds[i].Group == _group) {
                ma_sound_stop(&mSounds[i].Sound);
                ma_sound_uninit(&mSounds[i].Sound);
                mSounds[i].bActive = false;
                mFreeIndices.push_back(i);
            }
        }
    }

    void AudioService::Update() {
        // Clean up finished one-shot sounds
        for (uint32 i = 0; i < mSounds.size(); ++i) {
            if (!mSounds[i].bActive) {
                continue;
            }

            // If a non-looping sound has finished playing, reclaim the slot
            if (!mSounds[i].bLooping && ma_sound_at_end(&mSounds[i].Sound) == MA_TRUE) {
                ma_sound_uninit(&mSounds[i].Sound);
                mSounds[i].bActive = false;
                mFreeIndices.push_back(i);
            }
        }
    }

    // ============== Validation ==============

    bool AudioService::IsSoundValid(SoundHandle _handle) const {
        return GetSoundData(_handle) != nullptr;
    }

    // ============== Private ==============

    SoundData* AudioService::GetSoundData(SoundHandle _handle) {
        if (_handle.Index >= mSounds.size()) {
            return nullptr;
        }
        SoundData& data = mSounds[_handle.Index];
        if (!data.bActive || data.Generation != _handle.Generation) {
            return nullptr;
        }
        return &data;
    }

    const SoundData* AudioService::GetSoundData(SoundHandle _handle) const {
        if (_handle.Index >= mSounds.size()) {
            return nullptr;
        }
        const SoundData& data = mSounds[_handle.Index];
        if (!data.bActive || data.Generation != _handle.Generation) {
            return nullptr;
        }
        return &data;
    }

    float AudioService::CalculateEffectiveVolume(const SoundData& _data) const {
        float masterVol = mGroupVolumes[static_cast<int>(ESoundGroup::Master)];
        float groupVol  = mGroupVolumes[static_cast<int>(_data.Group)];
        return _data.Volume * groupVol * masterVol;
    }

    void AudioService::ReapplyVolume(SoundData& _data) {
        float effective = CalculateEffectiveVolume(_data);
        ma_sound_set_volume(&_data.Sound, effective);
    }

} // namespace Umbra
