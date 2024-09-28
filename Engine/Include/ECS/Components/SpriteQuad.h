#pragma once
#include "Asset/Texture.h"
#include "ECS/Component.h"
#include "Umbra.h"

#include "SFML/Graphics.hpp"
namespace Umbra {
    struct SpriteComponent : Component {
    public:
        inline SpriteComponent(sf::Color _color = sf::Color::White) {
            color = _color;
        }
        inline SpriteComponent(SharedPtr<Texture>& _refTexture, sf::Color _color = sf::Color::White)
            : color(_color), refTexture(_refTexture) {}
        sf::Color color;
        /* data */
        SharedPtr<Texture> refTexture;
    };
} // namespace Umbra
