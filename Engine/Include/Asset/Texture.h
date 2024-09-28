
#pragma once
#include "Asset/TextureResource.h"
#include "Umbra.h"

#include "SFML/Graphics.hpp"
namespace Umbra {
    class TextureResource : public IResource {
    public:
        TextureResource(const String& _filePath);
        //~TextureResource();

        bool Load() override;
        void Unload() override;
        sf::Texture SfmlTex;
    };
    class Texture {
    public:
        Texture(SharedPtr<TextureResource> _textureResource);
        const sf::Texture* GetSfmlTexture();

    private:
        SharedPtr<TextureResource> refTextureResource;
    };
} // namespace Umbra
