#pragma once
#include "EnginePCH.h"
#include "Math/Vector.h"

namespace Umbra {

    /// @brief Configuration for the physics service
    struct PhysicsServiceConfig {
        Math::Vector2f Gravity              = Math::Vector2f(0.0f, -98.0f);
        float LinearDamping                 = 0.975f; // Global damping applied to all bodies
        float AngularDamping                = 0.975f; // Global angular damping
        int VelocityIterations              = 8; // Solver iterations for velocity
        int PositionIterations              = 3; // Solver iterations for position
        float BaumgarteScale                = 0.2f; // Position correction factor
        uint32 InitialBodyCapacity          = 256; // Pre-allocated body slots
        float AABBFattenMargin              = 0.1f; // Fat AABB uniform expansion margin
        float AABBDisplacementMultiplier    = 2.0f; // Velocity-based AABB extension scale
        float LinearVelocitySleepThreshold  = 0.5f; // Velocity threshold under which  bodies can sleep
        float AngularVelocitySleepThreshold = 0.4f; // AngularVelocity threshold under which  bodies can sleep
        float SleepTimeThreshold            = 0.5f; // How long should a body be under threshold to sleep
        bool bEnableCCD                     = true; // Global toggle for continuous collision detection
        float CCDMotionThreshold            = 0.5f; // Minimum displacement (fraction of body extent) to trigger CCD
        int CCDBoxBisectionIterations       = 8; // Bisection steps for box-box TOI
    };

} // namespace Umbra
