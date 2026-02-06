#pragma once
#include "EnginePCH.h"
#include "Math/Vector.h"

namespace Umbra {

    /// @brief Configuration for the physics service
    struct PhysicsServiceConfig {
        Math::Vector2f Gravity        = Math::Vector2f(0.0f, -9.8f);
        float LinearDamping           = 0.975f;  // Global damping applied to all bodies
        float AngularDamping          = 0.975f;  // Global angular damping
        int VelocityIterations        = 8;       // Solver iterations for velocity
        int PositionIterations        = 3;       // Solver iterations for position
        float BaumgarteScale          = 0.2f;    // Position correction factor
        uint32 InitialBodyCapacity    = 256;     // Pre-allocated body slots
    };

} // namespace Umbra
