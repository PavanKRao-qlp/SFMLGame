#include "Asset/AssetManager.h"

namespace Umbra {
    AssetManager::AssetManager() {
        assetRegister = std::make_unique<AssetRegister>();
    }
    SharedPtr<Texture> AssetManager::GetTexture(const String& _filePath) {

        SharedPtr<Texture> refTexture = nullptr;
        //
        SharedPtr<TextureResource> refTextureResource = GetResource<TextureResource>(_filePath);
        if (refTextureResource != nullptr) {
            refTexture = std::make_shared<Texture>(refTextureResource);
        }
        return refTexture;
    }
    SharedPtr<Audio> AssetManager::GetAudio(const String& _filePath) {
        SharedPtr<Audio> refAudio = nullptr;
        SharedPtr<AudioResource> refAudioResource = GetResource<AudioResource>(_filePath);
        if (refAudioResource != nullptr) {
            refAudio = std::make_shared<Audio>(refAudioResource);
        }
        return refAudio;
    }
} // namespace Umbra
