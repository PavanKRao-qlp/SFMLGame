#pragma once
#include "ECS/Component.h"
namespace Umbra {
    struct LifeTimeComponent : Component {
    public:
        inline LifeTimeComponent(float _lifetime) {
            Time = _lifetime;
        }
        float Time;
        /* data */
    };
} // namespace Umbra
