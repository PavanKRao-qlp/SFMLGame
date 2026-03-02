#pragma once
#include "EnginePCH.h"
#include "Math/Vector.h"
#include "Service/Physics/PhysicsHandle.h"

namespace Umbra {

    /// @brief Types of physics constraints
    enum class EConstraintType : uint8 {
        Spring,
        Distance,
        Hinge
    };

    // ============== Constraint Definitions (user-facing) ==============

    /// @brief Definition for a spring constraint (Hooke's law + damping)
    /// Connects two bodies (or one body to a world anchor) with a force-based spring
    struct SpringDef {
        BodyHandle BodyA;                                     // First body (required)
        BodyHandle BodyB;                                     // Second body (Invalid() for world anchor)
        Math::Vector2f LocalAnchorA = Math::Vector2f(0, 0);   // Anchor in body A local space
        Math::Vector2f LocalAnchorB = Math::Vector2f(0, 0);   // Anchor in body B local space
        Math::Vector2f WorldAnchorB = Math::Vector2f(0, 0);   // World-space anchor when BodyB is invalid
        float Stiffness             = 100.0f;                 // Hooke's constant (N/m)
        float Damping               = 5.0f;                   // Damping coefficient (Ns/m)
        float RestLength            = 0.0f;                   // 0 = auto-calculate from initial positions
    };

    /// @brief Definition for a rigid distance constraint
    /// Maintains a fixed distance between two anchor points using velocity-level impulses
    struct DistanceDef {
        BodyHandle BodyA;
        BodyHandle BodyB;
        Math::Vector2f LocalAnchorA = Math::Vector2f(0, 0);
        Math::Vector2f LocalAnchorB = Math::Vector2f(0, 0);
        float Distance              = 0.0f;                   // 0 = auto-calculate from initial positions
    };

    /// @brief Definition for a hinge (revolute) joint
    /// Pins two bodies at a shared point with free rotation, optional angle limits and motor
    struct HingeDef {
        BodyHandle BodyA;
        BodyHandle BodyB;
        Math::Vector2f LocalAnchorA = Math::Vector2f(0, 0);   // Should map to same world point
        Math::Vector2f LocalAnchorB = Math::Vector2f(0, 0);
        bool bEnableLimits          = false;
        float LowerAngle            = 0.0f;                   // Radians
        float UpperAngle            = 0.0f;                   // Radians
        bool bEnableMotor           = false;
        float MotorSpeed            = 0.0f;                   // Target angular velocity (rad/s)
        float MaxMotorTorque        = 0.0f;                   // Maximum motor torque
    };

    // ============== Internal Constraint Data (solver state) ==============

    struct SpringConstraintData {
        BodyHandle HandleA;
        BodyHandle HandleB;
        Math::Vector2f LocalAnchorA;
        Math::Vector2f LocalAnchorB;
        Math::Vector2f WorldAnchorB;
        float Stiffness  = 0.0f;
        float Damping    = 0.0f;
        float RestLength = 0.0f;
        bool bIsActive   = true;
        uint32 Generation = 0;
    };

    struct DistanceConstraintData {
        BodyHandle HandleA;
        BodyHandle HandleB;
        Math::Vector2f LocalAnchorA;
        Math::Vector2f LocalAnchorB;
        float Distance = 0.0f;
        // Solver cache (precomputed per frame)
        Math::Vector2f rA;
        Math::Vector2f rB;
        Math::Vector2f Axis;          // Normalized constraint axis
        float EffectiveMass   = 0.0f;
        float ImpulseAccum    = 0.0f; // Accumulated impulse for warm-starting
        float Bias            = 0.0f; // Baumgarte position correction bias
        bool bIsActive        = true;
        uint32 Generation     = 0;
    };

    struct HingeConstraintData {
        BodyHandle HandleA;
        BodyHandle HandleB;
        Math::Vector2f LocalAnchorA;
        Math::Vector2f LocalAnchorB;
        bool bEnableLimits = false;
        float LowerAngle   = 0.0f;
        float UpperAngle   = 0.0f;
        bool bEnableMotor  = false;
        float MotorSpeed   = 0.0f;
        float MaxMotorTorque = 0.0f;
        // Solver cache
        Math::Vector2f rA;
        Math::Vector2f rB;
        // 2x2 effective mass matrix (stored as 4 floats: Kxx, Kxy, Kyx, Kyy)
        float Kxx = 0.0f;
        float Kxy = 0.0f;
        float Kyx = 0.0f;
        float Kyy = 0.0f;
        Math::Vector2f ImpulseAccum;       // Accumulated point constraint impulse (2 DOF)
        float AngleEffectiveMass = 0.0f;   // For angular limits/motor
        float AngleImpulseAccum  = 0.0f;   // Accumulated angle limit impulse
        float MotorImpulseAccum  = 0.0f;   // Accumulated motor impulse
        Math::Vector2f PositionBias;       // Position error bias
        bool bIsActive    = true;
        uint32 Generation = 0;
    };

} // namespace Umbra
