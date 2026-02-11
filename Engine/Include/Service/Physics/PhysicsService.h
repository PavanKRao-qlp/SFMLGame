#pragma once
#include "EnginePCH.h"
#include "Math/Vector.h"
#include "Service/Physics/Collision.h"
#include "Service/Physics/DynamicAABBTree.h"
#include "Service/Physics/PhysicsBody.h"
#include "Service/Physics/PhysicsHandle.h"
#include "Service/Physics/PhysicsServiceConfig.h"

namespace Umbra {

    /// @brief Physics service that manages physics simulation independent of ECS
    /// Bodies are accessed via handles, ECS components hold handles and sync with service
    class PhysicsService {
    public:
        PhysicsService();
        explicit PhysicsService(const PhysicsServiceConfig& _config);
        ~PhysicsService();

        // ============== Configuration ==============

        void SetGravity(Math::Vector2f _gravity);
        Math::Vector2f GetGravity() const;

        void SetLinearDamping(float _damping);
        float GetLinearDamping() const;

        void SetAngularDamping(float _damping);
        float GetAngularDamping() const;

        const PhysicsServiceConfig& GetConfig() const;
        PhysicsServiceConfig& GetConfig();

        // ============== Simulation ==============

        /// @brief Steps the physics simulation
        /// @param _deltaTime Fixed timestep
        void Step(float _deltaTime);

        // ============== Body Management ==============

        /// @brief Creates a new physics body
        /// @param _def Body definition
        /// @return Handle to the created body
        BodyHandle CreateBody(const BodyDef& _def);

        /// @brief Destroys a physics body
        /// @param _handle Handle to destroy
        void DestroyBody(BodyHandle _handle);

        /// @brief Checks if a body handle is valid
        /// @param _handle Handle to validate
        /// @return True if handle points to a valid body
        bool IsBodyValid(BodyHandle _handle) const;

        /// @brief Gets the number of active bodies
        uint32 GetBodyCount() const;

        // ============== Body State (Read) ==============

        Math::Vector2f GetPosition(BodyHandle _handle) const;
        float GetAngle(BodyHandle _handle) const;
        Math::Vector2f GetVelocity(BodyHandle _handle) const;
        float GetAngularVelocity(BodyHandle _handle) const;
        Math::Vector2f GetAcceleration(BodyHandle _handle) const;
        float GetAngularAcceleration(BodyHandle _handle) const;
        float GetMass(BodyHandle _handle) const;
        float GetInertia(BodyHandle _handle) const;
        void* GetUserData(BodyHandle _handle) const;
        bool IsKinematic(BodyHandle _handle) const;
        bool IsStatic(BodyHandle _handle) const;
        float GetCoefOfRestitution(BodyHandle _handle) const;
        float GetStaticFriction(BodyHandle _handle) const;
        float GetDynamicFriction(BodyHandle _handle) const;

        // ============== Body State (Write) ==============

        void SetPosition(BodyHandle _handle, Math::Vector2f _position);
        void SetAngle(BodyHandle _handle, float _angle);
        void SetVelocity(BodyHandle _handle, Math::Vector2f _velocity);
        void SetAngularVelocity(BodyHandle _handle, float _angularVelocity);
        void SetMass(BodyHandle _handle, float _mass);
        void SetCoefOfRestitution(BodyHandle _handle, float _coefOfRestitution);
        void SetStaticFriction(BodyHandle _handle, float _staticFriction);
        void SetDynamicFriction(BodyHandle _handle, float _dynamicFriction);
        void SetInertia(BodyHandle _handle, float _inertia);
        void SetUserData(BodyHandle _handle, void* _userData);
        void SetKinematic(BodyHandle _handle, bool _bIsKinematic);
        void SetAffectedByGravity(BodyHandle _handle, bool _bAffected);

        // ============== Force Application ==============

        /// @brief Applies force at center of mass (no torque)
        void ApplyForce(BodyHandle _handle, Math::Vector2f _force);

        /// @brief Applies force at a world point (generates torque)
        void ApplyForceAtPoint(BodyHandle _handle, Math::Vector2f _force, Math::Vector2f _worldPoint);

        /// @brief Applies force at a local point relative to body center
        void ApplyForceAtLocalPoint(BodyHandle _handle, Math::Vector2f _force, Math::Vector2f _localPoint);

        /// @brief Applies instantaneous impulse at center of mass
        void ApplyImpulse(BodyHandle _handle, Math::Vector2f _impulse);

        /// @brief Applies instantaneous impulse at a world point
        void ApplyImpulseAtPoint(
            BodyHandle _handle, Math::Vector2f _impulse, Math::Vector2f _worldPoint, bool _bShouldAwake = true);

        /// @brief Applies torque around center of mass
        void ApplyTorque(BodyHandle _handle, float _torque);

        /// @brief Applies angular impulse
        void ApplyAngularImpulse(BodyHandle _handle, float _impulse);

        // ============== Broadphase Access ==============

        const DynamicAABBTree& GetBroadphaseTree() const;
        uint32 GetBroadphaseChecks() const;
        uint32 GetBroadphasePairCount() const;

        // ============== Collision Queries ==============

        /// @brief Tests overlap between two bodies using their shapes and positions
        bool TestOverlap(BodyHandle _a, BodyHandle _b) const;

        /// @brief Returns the list of collisions detected during the last Step()
        const Vector<CollisionDef>& GetCollisions() const;

        /// @brief Returns collision events that started this frame
        const Vector<CollisionEvent>& GetCollisionEnterEvents() const;

        /// @brief Returns collision events that persisted from last frame
        const Vector<CollisionEvent>& GetCollisionStayEvents() const;

        /// @brief Returns collision pairs that ended this frame
        const Vector<CollisionEvent>& GetCollisionExitEvents() const;

        // ============== Spatial Queries ==============

        /// @brief Tests if a world-space point is inside any body's shape
        /// @param _point World-space point to test
        /// @return Handle to the first body found, or BodyHandle::Invalid() if none
        BodyHandle PointQuery(Math::Vector2f _point) const;

        /// @brief Casts a ray and returns the closest hit
        /// @param _origin Ray origin in world space
        /// @param _direction Ray direction (will be normalized internally)
        /// @param _maxDistance Maximum ray distance
        /// @param _hit Output hit result
        /// @return True if the ray hit a body
        bool Raycast(Math::Vector2f _origin, Math::Vector2f _direction, float _maxDistance, RaycastHit& _hit) const;

        /// @brief Casts a ray and returns all hits sorted by distance
        /// @param _origin Ray origin in world space
        /// @param _direction Ray direction (will be normalized internally)
        /// @param _maxDistance Maximum ray distance
        /// @return Vector of all hits sorted by distance (nearest first)
        Vector<RaycastHit> RaycastAll(
            Math::Vector2f _origin, Math::Vector2f _direction, float _maxDistance) const;

        // ============== Internal Access (for sync system) ==============

        /// @brief Gets direct access to body data (use with caution)
        /// @param _handle Body handle
        /// @return Pointer to body data or nullptr if invalid
        PhysicsBodyData* GetBodyData(BodyHandle _handle);
        const PhysicsBodyData* GetBodyData(BodyHandle _handle) const;

        /// @brief Gets the handle for a body data reference
        /// @param _bodyData Reference to a body data owned by this service
        /// @return Valid handle if the body is active, invalid handle otherwise
        BodyHandle GetBodyHandle(const PhysicsBodyData& _bodyData) const;

        /// @brief Iterates all active bodies (for sync system)
        template <typename Func>
        void ForEachBody(Func&& _func);

        template <typename Func>
        void ForEachBody(Func&& _func) const;

    private:
        void IntegrateForces(float _deltaTime);
        void IntegrateVelocities(float _deltaTime);
        void ApplyDamping(float _deltaTime);
        void ClearForceAccumulators();
        void UpdateBroadphaseProxies(float _deltaTime);
        void BroadphaseDetection();
        void NarrowPhaseDetection();
        void PrecomputeContactConstraints();
        void ResolveContacts();
        void PositionContraction();
        void UpdateSleepingBodies(float _deltaTime);

        PhysicsBodyData* GetBodyDataInternal(BodyHandle _handle);
        const PhysicsBodyData* GetBodyDataInternal(BodyHandle _handle) const;

        bool PointInBody(Math::Vector2f _point, const PhysicsBodyData& _body) const;
        bool RaycastBody(Math::Vector2f _origin, Math::Vector2f _direction, float _maxDistance,
            const PhysicsBodyData& _body, float& _outDistance, Math::Vector2f& _outNormal) const;

        static uint64 MakeCollisionPairKey(uint32 _indexA, uint32 _indexB);
        void CategorizeCollisionEvents();

    private:
        PhysicsServiceConfig mConfig;

        // Body storage
        Vector<PhysicsBodyData> mBodies;
        Vector<uint32> mFreeIndices; // Recycled slots
        uint32 mActiveBodyCount = 0;
        // collsion types
        Vector<Tuple<BodyHandle, BodyHandle>> mOverlappingBoundsIndexPair;
        Vector<CollisionDef> mCollisions;

        // Broadphase acceleration structure
        DynamicAABBTree mBroadphaseTree;
        uint32 mBroadphaseChecks    = 0;
        uint32 mBroadphasePairCount = 0;

        // Collision event tracking (enter/stay/exit)
        Set<uint64> mPreviousCollisionPairs;
        Vector<CollisionEvent> mCollisionEnterEvents;
        Vector<CollisionEvent> mCollisionStayEvents;
        Vector<CollisionEvent> mCollisionExitEvents;
    };

    // ============== Template Implementations ==============

    template <typename Func>
    void PhysicsService::ForEachBody(Func&& _func) {
        for (uint32 i = 0; i < mBodies.size(); ++i) {
            if (mBodies[i].bIsActive) {
                BodyHandle handle;
                handle.Index      = i;
                handle.Generation = mBodies[i].Generation;
                _func(handle, mBodies[i]);
            }
        }
    }

    template <typename Func>
    void PhysicsService::ForEachBody(Func&& _func) const {
        for (uint32 i = 0; i < mBodies.size(); ++i) {
            if (mBodies[i].bIsActive) {
                BodyHandle handle;
                handle.Index      = i;
                handle.Generation = mBodies[i].Generation;
                _func(handle, mBodies[i]);
            }
        }
    }

} // namespace Umbra
