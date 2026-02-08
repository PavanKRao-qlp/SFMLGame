#pragma once
#include "Service/Physics/PhysicsHandle.h"

namespace Umbra {

    struct ContactDef {
        Math::Vector2f contactPoint;
        Math::Vector2f contactNormal;
        float penetration = 0;
    };

    struct CollisionDef {
    public:
        int indexA;
        int indexB;
        Math::Vector2f contactNormal; // Points from body A toward body B
        float penetration = 0; // Overlap depth along the contact normal
        Vector<ContactDef> contacts;
    };

} // namespace Umbra
