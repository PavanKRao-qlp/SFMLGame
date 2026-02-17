#pragma once
#include "EnginePCH.h"
#include "Math/Bounds.h"
#include "Math/Vector.h"
#include "Service/Physics/Shape.h"

namespace Umbra {

    /// @brief Bitmask-based collision filter (Box2D-style)
    /// Two bodies A,B collide only if (A.Category & B.Mask) && (B.Category & A.Mask)
    struct CollisionFilter {
        uint16 CategoryBits = 0x0001; // What this body IS (default: layer 0)
        uint16 MaskBits     = 0xFFFF; // What this body collides WITH (default: all)
    };

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
        float CoefOfRestitution = 1.0f; // 0 = perfectly inelastic, 1 = perfectly elastic
        float StaticFriction    = 0.6f;
        float DynamicFriction   = 0.4f;
        ShapeData ShapeData;
        bool bAffectedByGravity = true;
        bool bIsKinematic       = false; // Kinematic bodies are moved by game code
        bool bCanSleep          = true;
        bool bIsTrigger         = false; // Trigger bodies detect overlap but skip physical response
        bool bEnableCCD         = false; // Enable continuous collision detection (bullet body)
        CollisionFilter Filter;
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

        // Restitution (bounciness)
        float CoefOfRestitution = 1.0f;

        // Friction
        float StaticFriction  = 0.6f;
        float DynamicFriction = 0.4f;

        // Sleep
        // If true, this body is sleeping (skipped in simulation for performance)
        // Sleeping bodies don't integrate motion or respond to collisions
        bool bIsSleeping = false;
        // Time the body has been below the sleep velocity threshold
        // Once this exceeds sleepTimeThreshold, the body can sleep
        float SleepTimer = 0.0f;


        // Flags
        bool bAffectedByGravity = true;
        bool bIsKinematic       = false;
        bool bIsActive          = true;
        // If true, this body is allowed to sleep when stationary
        // Set to false for bodies that should always be active (player, etc.)
        bool bCanSleep  = true;
        bool bIsTrigger = false; // Trigger bodies detect overlap but skip solver
        bool bEnableCCD = false; // Continuous collision detection enabled

        // CCD snapshot (pre-integration position for sweep tests)
        Math::Vector2f CCDSavedPosition;

        // Collision filtering
        CollisionFilter Filter;

        // Shape
        ShapeData BodyShape;
        Math::Bounds2D BoundingAABB;

        // User data for ECS bridge
        void* UserData = nullptr;

        // Handle validation
        uint32 Generation = 0;

        // Broadphase tree proxy
        int32 TreeProxyId = -1;

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
                float sizeX = BodyShape.GetBox().GetSize().x;
                float sizeY = BodyShape.GetBox().GetSize().y;
                Math::Vector2f AABBSize =
                    Math::Vector2f(Math::Abs(sizeX * Math::Cos(Angle)) + Math::Abs(sizeY * Math::Sin(Angle)),
                        Math::Abs(sizeX * Math::Sin(Angle)) + Math::Abs(sizeY * Math::Cos(Angle)));
                BoundingAABB = Math::Bounds2D(Math::Vector2f(0, 0), AABBSize);
            }
        }

        // Put the body to sleep (stops simulation)
        inline void Sleep() {
            if (!bCanSleep || IsStatic()) {
                return;
            }
            bIsSleeping       = true;
            Velocity          = Math::Vector2f(0, 0);
            AngularVelocity   = 0.0f;
            ForceAccumulated  = Math::Vector2f(0, 0);
            TorqueAccumulated = 0.0f;
        }

        // Wake the body up (resumes simulation)
        inline void Wake() {
            bIsSleeping = false;
            SleepTimer  = 0.0f;
        }
    };

} // namespace Umbra
