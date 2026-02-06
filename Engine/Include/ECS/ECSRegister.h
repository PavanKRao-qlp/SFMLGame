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

        void RemoveComponent(EntityID _entity, ComponentID _componentId);

        template <typename T>
        bool HasComponent(EntityID _entity);

        template <typename T>
        T* GetComponent(EntityID _entity);

        void AddSystem(ESystemPhase _systemPhase, int _priority, SharedPtr<System> _system);
        void RemoveSystem(SharedPtr<System>& _system);

        void Update();
        void Update(ESystemPhase _systemPhase);
        void CleanUp();

    private:
        void RemoveDestroyedEntities();
        void AddCreatedEntities();
        EntityManager mEntityManager;
        UniquePtr<ComponentManager> mComponentManager;

        UMap<EntityID, ComponentMask> mEntityComponentSignatures;
        UMap<ESystemPhase, Vector<SharedPtr<System>>> mSystemMap;

        // Tag index for efficient tag-based entity queries
        UMap<String, Set<EntityID>> mTagIndex;
        Set<EntityID> mEmptyEntitySet; // Returned when querying non-existent tags

        bool bRegisterDirty = true;
    };

} // namespace Umbra

#include "ECSRegister.inl"
