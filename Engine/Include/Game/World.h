#pragma once
#include "ECS/ECSRegister.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/RenderSystem.h"
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

        PhysicsSystem* GetPhysicsSystem();

        Math::Vector2f GetScreenToWorldPosition(Math::Vector2i _screenPos);

    private:
        SharedPtr<ECSRegister> mWorldRegister;
        SharedPtr<RenderSystem> mRenderSystem;
        SharedPtr<CameraSystem> mCameraSystem;
        SharedPtr<PhysicsSystem> mPhysicsSystem;
    };

} // namespace Umbra

#include "Game/World.inl"
