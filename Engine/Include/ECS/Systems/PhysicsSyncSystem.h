#pragma once
// ECSRegister.h must be included before System.h to resolve their circular dependency
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/Transform.h"
#include "ECS/ECSRegister.h"
#include "ECS/System.h"
#include "Service/Physics/PhysicsService.h"

namespace Umbra {

    /// @brief System that bridges ECS entities with the PhysicsService
    /// Handles body creation, destruction, and transform synchronization
    class PhysicsSyncSystem : public System {
    public:
        explicit PhysicsSyncSystem(PhysicsService* _physicsService);
        ~PhysicsSyncSystem();

        void Update() override;

        /// @brief Called when an entity with RigidbodyHandleComponent is added
        void AddEntity(EntityID _entity) override;

        /// @brief Called when an entity with RigidbodyHandleComponent is removed
        void RemoveEntity(EntityID _entity) override;

        /// @brief Access to physics service for game code
        PhysicsService* GetPhysicsService() const {
            return mPhysicsService;
        }

    private:
        /// @brief Creates a physics body for an entity that needs one
        void CreateBodyForEntity(EntityID _entity);

        /// @brief Destroys a physics body when entity is removed
        void DestroyBodyForEntity(EntityID _entity);

        /// @brief Syncs kinematic entity transforms TO physics service
        void SyncKinematicToPhysics();

        /// @brief handles unreal changes of rigidbody properties such as change in mass / inertia
        void SyncRigidbodyPropertyChanges();

        /// @brief Syncs dynamic entity transforms FROM physics service
        void SyncPhysicsToDynamic();

    private:
        PhysicsService* mPhysicsService = nullptr;
    };

} // namespace Umbra
