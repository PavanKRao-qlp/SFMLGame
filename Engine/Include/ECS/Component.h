#pragma once
#include "EnginePCH.h"
#include "ECS/ECSConfig.h"

namespace D2D
{

    class ComponentIDHelper
    {
    public:
        template <typename T>
        inline static ComponentID GetID()
        {
            static ComponentID typeID{getUniqueComponentID()};
            return typeID;
        };

    private:
        inline static ComponentID getUniqueComponentID()
        {
            // states that the static ComponentID is going to start at 1
            static ComponentID lastID{1u};
            return lastID++;
        }
    };

    struct Component
    {
    };

    class BaseComponentArray
    {
    };

    template <typename Component, typename... Rest>
    ComponentMask CreateSignature()
    {
        ComponentMask signature;
        ComponentID id = ComponentIDHelper::GetID<Component>();
        signature.set(id);
        if constexpr (sizeof...(Rest) > 0)
        {
            signature |= CreateSignature<Rest...>();
        }
        return signature;
    }

    template <typename T>
    class ComponentArray : public BaseComponentArray
    {
    private:
        /* data */
    public:
        ComponentArray();
        ~ComponentArray();
        T &Get(EntityID _entity);
        void Insert(EntityID _entity, T _component);
        void Remove(EntityID _entity, T _component);
        bool Has(EntityID _entity);

    protected:
        Vector<T> mPackedComponents;
        UMap<EntityID, int> mSparseIndexMap;
    };

    class ComponentArrayPool
    {
    public:
        template <typename T>
        inline ComponentArray<T> *GetComponentArray()
        {
            ComponentID id = ComponentIDHelper::GetID<T>();
            return static_cast<ComponentArray<T> *>(mComponentArrays[id]);
        };

        template <typename T>
        inline void AddComponentArray(ComponentArray<T> *_array)
        {
            ComponentID id = ComponentIDHelper::GetID<T>();
            mComponentArrays.insert_or_assign(id, _array);
        };

        template <typename T>
        inline void RemoveComponentArray()
        {
            ComponentID id = ComponentIDHelper::GetID<T>();
            mComponentArrays.erase(id);
        };

        inline void RemoveComponentArray(ComponentID _id) {
            mComponentArrays.erase(_id);
            // mComponentArrays.remove(_id);
        };

        template <typename T>
        inline bool HasComponentArray()
        {
            ComponentID id = ComponentIDHelper::GetID<T>();
            return mComponentArrays.find(id) != mComponentArrays.end();
        };

        inline bool HasComponentArray(ComponentID _id)
        {
            // return false;
            return mComponentArrays.find(_id) != mComponentArrays.end();
        };

    protected:
        UMap<ComponentID, BaseComponentArray *> mComponentArrays;
    };

    class ComponentManager
    {
    };
}

#include "Component.inl"
