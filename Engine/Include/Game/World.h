#pragma once
#include "Diag/MemoryTracker.h"
#include "ECS/ECSRegister.h"
#include "ECS/Systems/AudioSyncSystem.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/CollisionEventDispatchSystem.h"
#include "ECS/Systems/PhysicsSyncSystem.h"
// #include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/AnimationSystem.h"
#include "ECS/Systems/RenderSyncSystem.h"
#include "ECS/Systems/SceneGraphSystem.h"
#include "Service/Audio/AudioService.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"

namespace Umbra {
    class World {
    public:
        void InitializeCoreSystems();
        // void SetECSRegister(ECSRegister* _worldRegister);
        // ECSRegister* GetRegister();
        // void FlushWorld();
        World();
        ~World();
        void Update();
        void Simulate();
        void Render();

        /**
         * @brief  Creates a new entity
         * @return Handle for created entity
         */
        const EntityID CreateEntity();

        void DestroyEntity(EntityID _entity);

        template <typename T>
        void RegisterComponent();

        template <typename T>
        void AddComponent(EntityID _entity, T _component);

        void AddTag(EntityID _entity, const String& _tag);

        bool IsTag(EntityID _entity, String _tag);

        void RemoveTag(EntityID _entity);

        /** Returns all entities with the specified tag. */
        Vector<EntityID> FindEntitiesByTag(const String& _tag);

        /** Returns a reference to the set of entities with the specified tag.
         *  Returns an empty set if no entities have the tag. */
        const Set<EntityID>& GetEntitiesByTag(const String& _tag);

        /** Returns true if any entity has the specified tag. */
        bool HasEntitiesWithTag(const String& _tag);

        template <typename T, typename... Args>
        void AddComponent(EntityID _entity, Args&&... args);

        template <typename T>
        void RemoveComponent(EntityID _entity);

        template <typename T>
        bool HasComponent(EntityID _entity);

        template <typename T>
        T* GetComponent(EntityID _entity);

        void AddSystem(ESystemPhase _phase, int _priority, SharedPtr<System> _system);
        void RemoveSystem(SharedPtr<System>& _system);

        /** Returns the first registered system of type T, or nullptr if not found. */
        template <typename T>
        SharedPtr<T> GetSystem();

        /** Enables or disables the first registered system of type T. */
        template <typename T>
        void SetSystemEnabled(bool _bEnabled);

        // PhysicsSystem* GetPhysicsSystem(); // removed — use GetPhysicsService()

        Math::Vector2f GetScreenToWorldPosition(Math::Vector2i _screenPos);

        // ============== Physics Service Layer ==============

        /// @brief Get the physics service for this world
        PhysicsService* GetPhysicsService();

        /// @brief Apply force to an entity through the physics service
        void ApplyForce(EntityID _entity, Math::Vector2f _force);

        /// @brief Applies force at a world point (generates torque) through the physics service
        void ApplyForceAtPoint(EntityID _entity, Math::Vector2f _force, Math::Vector2f _worldPoint);

        /// @brief Apply impulse to an entity through the physics service
        void ApplyImpulse(EntityID _entity, Math::Vector2f _impulse);

        /// @brief Apply torque to an entity through the physics service
        void ApplyTorque(EntityID _entity, float _torque);

        /// @brief Applies angular impulse to an entity through the physics service
        void ApplyAngularImpulse(EntityID _entity, float _impulse);

        /// @brief Set velocity of an entity through the physics service
        void SetVelocity(EntityID _entity, Math::Vector2f _velocity);

        /// @brief Get velocity of an entity (uses cached value for efficiency)
        Math::Vector2f GetVelocity(EntityID _entity);

        // ============== Audio Service Layer ==============

        /// @brief Get the audio service for this world
        AudioService* GetAudioService();

        /// @brief Play a sound (fire-and-forget convenience)
        void PlaySound(const String& _filePath, ESoundGroup _group = ESoundGroup::SFX);

        /// @brief Set volume for a sound group
        void SetGroupVolume(ESoundGroup _group, float _volume);

        // ============== Scene Graph Layer ==============

        /// @brief Attach _child to _parent in the scene hierarchy.
        ///        Detaches _child from its current parent first if it has one.
        ///        Both entities must have a TransformComponent.
        void SetParent(EntityID _child, EntityID _parent);

        /// @brief Remove _child from its current parent, making it a root entity.
        void DetachFromParent(EntityID _child);

        /// @brief Returns the parent of _entity, or MAX_ENTITY if it is a root.
        EntityID GetParent(EntityID _entity) const;

        /// @brief Returns the direct children of _entity.
        const Vector<EntityID>& GetChildren(EntityID _entity) const;

        // ============== Prefab Layer ==============

        /// @brief Instantiate a registered prefab by name. Returns MAX_ENTITY if not found.
        EntityID Instantiate(const String& _prefabName);

        /// @brief Instantiate a registered prefab by name, then run an override on the new entity.
        EntityID Instantiate(const String& _prefabName, std::function<void(EntityID)> _overrideFn);

    private:
        SharedPtr<ECSRegister> mWorldRegister;
        SharedPtr<SceneGraphSystem> mSceneGraphSystem;
        SharedPtr<AnimationSystem> mAnimationSystem;
        SharedPtr<RenderSyncSystem> mRenderSyncSystem;
        SharedPtr<CameraSystem> mCameraSystem;
        // SharedPtr<PhysicsSystem> mPhysicsSystem;
        SharedPtr<PhysicsSyncSystem> mPhysicsSyncSystem;
        SharedPtr<CollisionEventDispatchSystem> mCollisionEventDispatchSystem;

        // Physics Service Layer
        UniquePtr<PhysicsService> mPhysicsService;

        // Audio Service Layer
        UniquePtr<AudioService> mAudioService;
        SharedPtr<AudioSyncSystem> mAudioSyncSystem;
    };

} // namespace Umbra

#include "Game/World.inl"
