#pragma once
#include "ECS/ECSRegister.h"
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
        void Update();

        /** Creates an entity. */
        const EntityID& CreateEntity();
        /** Marks Entity as Destroyed.*/
        void DestroyEntity(EntityID _entity);

        template <typename T>
        void AddComponent(EntityID _entity, T _component);

        void AddTag(EntityID _entity, const String& _tag);

        bool IsTag(EntityID _entity, String _tag);

        template <typename T, typename... Args>
        void AddComponent(EntityID _entity, Args&&... args);

        template <typename T>
        void RemoveComponent(EntityID _entity);

        template <typename T>
        bool HasComponent(EntityID _entity);

        template <typename T>
        T* GetComponent(EntityID _entity);

        void AddSystem(SharedPtr<System> _system);
        void RemoveSystem(SharedPtr<System>& _system);


    private:
        SharedPtr<ECSRegister> mWorldRegister;
        SharedPtr<RenderSystem> mRenderSystem;
    };

} // namespace Umbra
