#pragma once
#include "Graphics/IFont.h"

#include <SFML/Graphics/Font.hpp>

namespace Umbra {

    class SfmlFont : public IFont {
    public:
        SfmlFont()  = default;
        ~SfmlFont() = default;

        bool LoadFromFile(const String& _path) override;
        void Unload() override;
        void* GetNativeHandle() const override;

        const sf::Font& GetSfmlFont() const { return mFont; }

    private:
        sf::Font mFont;
    };

} // namespace Umbra
