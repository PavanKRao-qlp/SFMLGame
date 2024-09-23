#pragma once
#include "ECS/Component.h"
#include "Math/Vector.h"
namespace UMBRA
{
    struct TransformComponent : Component
    {
    public:
        inline TransformComponent(MATH::Vector2f _position, MATH::Vector2f _size, float _angle = 0, MATH::Vector2f _pivot = MATH::Vector2f(0.5f, 0.5f)) : 
        Position(_position)
        ,Size(_size)
        ,Pivot(_pivot)
        ,Angle(_angle)
        {};
        MATH::Vector2f Position;
        MATH::Vector2f Size;
        MATH::Vector2f Pivot;
        float Angle = 0;
        /* data */
    };
}