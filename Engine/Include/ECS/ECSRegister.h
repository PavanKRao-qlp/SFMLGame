#pragma once
#include "ECS/Component.h"
#include "ECS/Components/Tag.h"
#include "ECS/ECSConfig.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "ECS/View.h"
#include "EnginePCH.h"
#include "Types/SparseArray.h"

namespace Umbra {
    class ECSRegister {
    public:
        ECSRegister();
        ~ECSRegister();
        /** Creates an entity. */
        EntityID CreateEntity();
        /** Marks Entity as Destroyed.*/
        void DestroyEntity(EntityID _entity);
        void FlushRegister();

        template <typename T>
        void RegisterComponent();

        template <typename T>
        void AddComponent(EntityID _entity, T _component);

        void AddTag(EntityID _entity, const String& _tag);

        bool IsTag(EntityID _entity, String _tag);

        template <typename T, typename... Args>
        void AddComponent(EntityID _entity, Args&&... args);

        template <typename T>
        void RemoveComponent(EntityID _entity);

        void RemoveComponent(EntityID _entity, ComponentID _componentId);

        template <typename T>
        bool HasComponent(EntityID _entity);

        template <typename T>
        T* GetComponent(EntityID _entity);

        void AddSystem(System* _system);
        void RemoveSystem(System* _system);

        void Update();

    private:
        void RemoveDestroyedEntities();
        void AddCreatedEntities();
        EntityManager mEntityManager;
        ComponentManager* mComponentManager;

        UMap<EntityID, ComponentMask> mEntityComponentSignatures;
        Vector<SharedPtr<System>> mSystems;

        bool bRegisterDirty = true;
    };

} // namespace Umbra

#include "ECSRegister.inl"
