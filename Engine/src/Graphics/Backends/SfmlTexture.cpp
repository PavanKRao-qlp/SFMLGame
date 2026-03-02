#include "Graphics/Backends/SfmlTexture.h"

namespace Umbra {

    bool SfmlTexture::LoadFromFile(const String& _path) {
        return mTexture.loadFromFile(_path);
    }

    void SfmlTexture::Unload() {
        mTexture = sf::Texture();
    }

    void* SfmlTexture::GetNativeHandle() const {
        return const_cast<sf::Texture*>(&mTexture);
    }

    Math::Vector2i SfmlTexture::GetSize() const {
        sf::Vector2u size = mTexture.getSize();
        return Math::Vector2i(static_cast<int>(size.x), static_cast<int>(size.y));
    }

} // namespace Umbra
