#include "Asset/Texture.h"
namespace Umbra {
    Texture::Texture(SharedPtr<TextureResource> _textureResource) : refTextureResource(_textureResource) {}

    const sf::Texture* Texture::GetSfmlTexture() {
        return &refTextureResource.get()->SfmlTex;
    }

    bool TextureResource::Load() {
        SfmlTex = sf::Texture();
        return SfmlTex.loadFromFile(filePath);
    }
    void TextureResource::Unload() {}
    TextureResource::TextureResource(const String& _filePath) : IResource(_filePath) {}
} // namespace Umbra
