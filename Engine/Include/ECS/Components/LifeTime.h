#pragma once
#include "ECS/Component.h"
namespace UMBRA
{
    struct LifeTimeComponent : Component
    {
    public:
        inline LifeTimeComponent(float _lifetime)
        {
            Time = _lifetime;
        }
        float Time;
        /* data */
    };
}