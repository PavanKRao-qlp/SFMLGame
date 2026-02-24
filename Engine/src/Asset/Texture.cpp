#include "Asset/Texture.h"

#include "Graphics/Backends/SfmlTexture.h"

namespace Umbra {
    Texture::Texture(SharedPtr<TextureResource> _textureResource) : refTextureResource(_textureResource) {}

    void* Texture::GetNativeHandle() {
        if (refTextureResource && refTextureResource->mTexture) {
            return refTextureResource->mTexture->GetNativeHandle();
        }
        return nullptr;
    }

    Math::Vector2i Texture::GetSize() const {
        if (refTextureResource && refTextureResource->mTexture) {
            return refTextureResource->mTexture->GetSize();
        }
        return Math::Vector2i(0, 0);
    }

    bool TextureResource::Load() {
        mTexture = std::make_unique<SfmlTexture>();
        return mTexture->LoadFromFile(filePath);
    }

    void TextureResource::Unload() {
        if (mTexture) {
            mTexture->Unload();
        }
    }

    TextureResource::TextureResource(const String& _filePath) : IResource(_filePath) {}
} // namespace Umbra
