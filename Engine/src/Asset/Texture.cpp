#include "Asset/Texture.h"

#include "Graphics/Backends/SfmlTexture.h"

namespace Umbra {
    Texture::Texture(SharedPtr<TextureResource> _textureResource)
        : IResourceHandle<TextureResource>(std::move(_textureResource)) {}

    void* Texture::GetNativeHandle() const {
        if (mResource && mResource->mTexture) {
            return mResource->mTexture->GetNativeHandle();
        }
        return nullptr;
    }

    Math::Vector2i Texture::GetSize() const {
        if (mResource && mResource->mTexture) {
            return mResource->mTexture->GetSize();
        }
        return Math::Vector2i(0, 0);
    }

    bool TextureResource::Load() {
        mTexture = std::make_unique<SfmlTexture>();
        bLoaded  = mTexture->LoadFromFile(filePath);
        return bLoaded;
    }

    void TextureResource::Unload() {
        if (mTexture) {
            mTexture->Unload();
        }
    }

    TextureResource::TextureResource(const String& _filePath) : IResource(_filePath) {}
} // namespace Umbra
