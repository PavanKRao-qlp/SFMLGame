#pragma once
#include "EnginePCH.h"
#include "Math/Bounds.h"
#include "Math/Vector.h"
#include "Service/Physics/Shape.h"

namespace Umbra {

    /// @brief Definition used to create a physics body
    struct BodyDef {
        Math::Vector2f Position = Math::Vector2f(0, 0);
        float Angle             = 0.0f;
        Math::Vector2f Velocity = Math::Vector2f(0, 0);
        float AngularVelocity   = 0.0f;
        float Mass              = 1.0f; // 0 = static/infinite mass
        float Inertia           = 0.0f; // 0 = auto-calculate or infinite
        float LinearDamping     = 0.0f;
        float AngularDamping    = 0.0f;
        ShapeData ShapeData;
        bool bAffectedByGravity = true;
        bool bIsKinematic       = false; // Kinematic bodies are moved by game code

        void* UserData = nullptr; // Opaque pointer (can store EntityID)
    };

    /// @brief Internal physics body data stored in SOA layout within PhysicsService
    /// This is not exposed directly - access through PhysicsService API
    struct PhysicsBodyData {
        // Transform
        Math::Vector2f Position;
        float Angle = 0.0f;

        // Velocity
        Math::Vector2f Velocity;
        float AngularVelocity = 0.0f;

        // Acceleration (computed each frame)
        Math::Vector2f Acceleration;
        float AngularAcceleration = 0.0f;

        // Force accumulators (cleared each frame)
        Math::Vector2f ForceAccumulated;
        float TorqueAccumulated = 0.0f;

        // Mass properties (inverse for efficiency)
        float InverseMass    = 1.0f; // 0 = static
        float InverseInertia = 1.0f;

        // Damping
        float LinearDamping  = 0.0f;
        float AngularDamping = 0.0f;

        // Flags
        bool bAffectedByGravity = true;
        bool bIsKinematic       = false;
        bool bIsActive          = true;

        // Shape
        ShapeData BodyShape;
        Math::Bounds2D BoundingAABB;

        // User data for ECS bridge
        void* UserData = nullptr;

        // Handle validation
        uint32 Generation = 0;

        inline bool IsStatic() const {
            return InverseMass <= 0.0f;
        }

        inline void SetMass(float _mass) {
            if (_mass > 0.0f) {
                InverseMass = 1.0f / _mass;
            } else {
                InverseMass = 0.0f;
            }
        }

        inline void SetInertia(float _inertia) {
            if (_inertia > 0.0f) {
                InverseInertia = 1.0f / _inertia;
            } else {
                InverseInertia = 0.0f;
            }
        }

        inline void UpdateBoundingAABB() {
            if (BodyShape.IsCircle()) {
                float size   = BodyShape.GetCircle().GetRadius() * 2;
                BoundingAABB = Math::Bounds2D(Math::Vector2f(0, 0), Math::Vector2f(size, size));
            }
            if (BodyShape.IsBox()) {
                float size   = BodyShape.GetBox().GetSize().Magnitude();
                BoundingAABB = Math::Bounds2D(Math::Vector2f(0, 0), Math::Vector2f(size, size));
            }
        }
    };

} // namespace Umbra
