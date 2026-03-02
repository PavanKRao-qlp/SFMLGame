#pragma once
#include "Asset/IResource.h"
#include "Service/Audio/AudioServiceConfig.h"
#include "Umbra.h"

namespace Umbra {

    class AudioResource : public IResource {
    public:
        AudioResource(const String& _filePath);

        bool Load() override;
        void Unload() override;

        const String& GetFilePath() const { return filePath; }
    };

    class Audio : public IResourceHandle<AudioResource> {
    public:
        explicit Audio(SharedPtr<AudioResource> _audioResource);

        const String& GetFilePath() const;
    };

} // namespace Umbra
