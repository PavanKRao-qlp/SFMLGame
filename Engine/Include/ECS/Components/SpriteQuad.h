#include "ECS/Component.h"
namespace D2D
{
    struct SpriteComponent : Component
    {
    public:
        inline SpriteComponent(sf::Color _color)
        {
            color = _color;
        }
        sf::Color color;
        /* data */
    };
}