#pragma once
#include "Service/Physics/PhysicsHandle.h"

namespace Umbra {

    struct ContactDef {
        Math::Vector2f contactPoint;
        Math::Vector2f contactNormal;
        float penetration = 0;

        // Solver cache (precomputed once per frame before iterations)
        Math::Vector2f rA;
        Math::Vector2f rB;
        float normalMass  = 0.0f;
        float tangentMass = 0.0f;
        float velocityBias = 0.0f;

        // Accumulated impulses (persist across solver iterations within a frame)
        float normalImpulseAccum  = 0.0f;
        float tangentImpulseAccum = 0.0f;
    };

    struct CollisionDef {
    public:
        BodyHandle handleA;
        BodyHandle handleB;
        Math::Vector2f contactNormal; // Points from body A toward body B
        float penetration = 0; // Overlap depth along the contact normal
        Vector<ContactDef> contacts;
    };

} // namespace Umbra
