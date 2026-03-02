#pragma once
#include "Graphics/ITexture.h"

#include <SFML/Graphics/Texture.hpp>

namespace Umbra {

    class SfmlTexture : public ITexture {
    public:
        SfmlTexture()  = default;
        ~SfmlTexture() = default;

        bool LoadFromFile(const String& _path) override;
        void Unload() override;
        void* GetNativeHandle() const override;
        Math::Vector2i GetSize() const override;

        const sf::Texture& GetSfmlTexture() const { return mTexture; }

    private:
        sf::Texture mTexture;
    };

} // namespace Umbra
