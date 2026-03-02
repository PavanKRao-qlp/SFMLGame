#include "Asset/AudioResource.h"

#include "Diag/Logger.h"

#include <miniaudio.h>

namespace Umbra {

    AudioResource::AudioResource(const String& _filePath) : IResource(_filePath) {}

    bool AudioResource::Load() {
        // Validate the audio file can be decoded by miniaudio
        ma_decoder decoder;
        ma_result result = ma_decoder_init_file(filePath.c_str(), nullptr, &decoder);
        if (result != MA_SUCCESS) {
            UMBRA_LOG_ERROR("Failed to load audio resource '{}' (error: {})", filePath, static_cast<int>(result));
            bLoaded = false;
            return false;
        }
        ma_decoder_uninit(&decoder);
        bLoaded = true;
        return true;
    }

    void AudioResource::Unload() {
        bLoaded = false;
    }

    Audio::Audio(SharedPtr<AudioResource> _audioResource)
        : IResourceHandle<AudioResource>(std::move(_audioResource)) {}

    const String& Audio::GetFilePath() const {
        return mResource->GetFilePath();
    }

} // namespace Umbra
