#pragma once
#include "ECS/Component.h"

#include "SFML/Graphics.hpp"
namespace Umbra {
    struct SpriteComponent : Component {
    public:
        inline SpriteComponent(sf::Color _color) {
            color = _color;
        }
        sf::Color color;
        /* data */
    };
} // namespace Umbra
