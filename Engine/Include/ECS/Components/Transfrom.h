#pragma once
#include "ECS/Component.h"
#include "Math/Vector.h"
namespace Umbra {
    struct TransformComponent : Component {
    public:
        inline TransformComponent(Math::Vector2f _position, Math::Vector2f _size, float _angle = 0,
            Math::Vector2f _pivot = Math::Vector2f(0.5f, 0.5f))
            : Position(_position), Size(_size), Pivot(_pivot), Angle(_angle) {};
        Math::Vector2f Position;
        Math::Vector2f Size;
        Math::Vector2f Pivot;
        float Angle = 0;
        /* data */
    };
} // namespace Umbra
