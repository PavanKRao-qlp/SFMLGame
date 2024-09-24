#pragma once
#include "ECS/Component.h"
#include "ECS/ECSConfig.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "ECS/View.h"
#include "EnginePCH.h"
#include "Types/SparseArray.h"

namespace Umbra {
    class ECSRegister {
    public:
        ECSRegister(/* args */);
        ~ECSRegister();
        /** Creates an entity. */
        EntityID CreateEntity();
        /** Destroys an entity.*/
        void DestroyEntity(EntityID _entity);

        template <typename T>
        void RegisterComponent();

        template <typename T>
        void AddComponent(EntityID _entity, T _component);

        template <typename T, typename... Args>
        void AddComponent(EntityID _entity, Args&&... args);

        template <typename T>
        void RemoveComponent(EntityID _entity);

        template <typename T>
        bool HasComponent(EntityID _entity);

        template <typename T>
        T* GetComponent(EntityID _entity);

        void AddSystem(System* _system);

        void Update();

    private:
        EntityManager mEntityManager;
        ComponentManager mComponentManager;
        SystemManager mSystemManager;

        UMap<EntityID, ComponentMask> mEntityComponentSignatures;
        ComponentArrayPool* ComponentArrayHolder;
        Vector<System*> mSystems;

        bool bRegisterDirty = true;
    };

} // namespace Umbra

#include "ECSRegister.inl"
