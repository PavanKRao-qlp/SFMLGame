#pragma once
#include "ECS/Enity.h"
#include "Umbra.h"

namespace Umbra {
    class Contact {
        EntityID mBodyA;
        EntityID mBodyB;
        Math::Vector2f mContactNormal;
        float mCofOfRestitution;
    };
} // namespace Umbra
