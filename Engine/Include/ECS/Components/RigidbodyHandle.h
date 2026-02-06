#pragma once
#include "ECS/Component.h"
#include "Math/Vector.h"
#include "Service/Physics/PhysicsHandle.h"

namespace Umbra {

    /// @brief ECS Component that bridges an entity to the PhysicsService
    /// Holds a handle into the physics service and cached state for efficient reads
    struct RigidbodyHandleComponent : public Component {
    public:
        // Handle into PhysicsService
        BodyHandle Handle;

        // Cached values (synced each frame from PhysicsService)
        // Use these for frequent reads to avoid handle lookups
        Math::Vector2f CachedVelocity;
        float CachedAngularVelocity = 0.0f;

        // Configuration (used when creating body in PhysicsService)
        float Mass              = 1.0f;
        float Inertia           = 0.0f;
        float LinearDamping     = 0.0f;
        float AngularDamping    = 0.0f;
        bool bAffectedByGravity = true;
        bool bIsKinematic       = false; // Kinematic = moved by game code, not physics

        // Sync flags
        bool bNeedsBodyCreation = true; // Set to true when component is added
        bool bSyncEnabled       = true; // Can disable sync for manual control

        inline bool HasValidBody() const {
            return Handle.IsValid() && !bNeedsBodyCreation;
        }
    };

} // namespace Umbra
