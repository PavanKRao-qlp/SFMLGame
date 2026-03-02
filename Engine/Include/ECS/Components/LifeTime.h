#pragma once
#include "ECS/Component.h"
namespace Umbra {
    struct LifeTimeComponent : Component {
    public:
        inline LifeTimeComponent(double _lifetime) {
            Time      = _lifetime;
            TimeSpent = 0;
        }
        double Time;
        double TimeSpent = 0;
        /* data */
    };
} // namespace Umbra
