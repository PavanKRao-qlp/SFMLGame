#pragma once
#include "ECS/Component.h"
namespace D2D
{
    struct TransformComponent : Component
    {
    public:
        inline TransformComponent(float _x, float _y, float _angle = 0)
        {
            x = _x;
            y = _y;
            angle = _angle;
        }
        float x = 0;
        float y = 0;
        float pivotX = 0.5f;
        float pivotY = 0.5f;
        float angle = 0;
        /* data */
    };
}