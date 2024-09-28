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
} // namespace Umbra
