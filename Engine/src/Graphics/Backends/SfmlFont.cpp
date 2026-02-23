#include "Graphics/Backends/SfmlFont.h"

namespace Umbra {

    bool SfmlFont::LoadFromFile(const String& _path) {
        return mFont.loadFromFile(_path);
    }

    void SfmlFont::Unload() {
        mFont = sf::Font();
    }

    void* SfmlFont::GetNativeHandle() const {
        return const_cast<sf::Font*>(&mFont);
    }

} // namespace Umbra
