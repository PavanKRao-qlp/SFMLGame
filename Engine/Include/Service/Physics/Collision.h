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
        bool bIsTrigger   = false; // True if either body is a trigger
        Vector<ContactDef> contacts;
    };

    struct CollisionEvent {
        BodyHandle HandleA;
        BodyHandle HandleB;
        Math::Vector2f ContactNormal; // Zero for exit events
        float Penetration = 0.0f;    // Zero for exit events
        bool bIsTrigger   = false;   // True if this was a trigger overlap
    };

    struct RaycastHit {
        BodyHandle Handle;
        Math::Vector2f Point;   // World-space hit point
        Math::Vector2f Normal;  // Surface normal at hit point
        float Distance = 0.0f;  // Distance along ray from origin
    };

} // namespace Umbra
