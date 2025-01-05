#pragma once
#include "ECS/ECSConfig.h"
#include "EnginePCH.h"

namespace Umbra {

    class ComponentIDHelper {
    public:
        template <typename T>
        inline static ComponentID GetID() {
            static ComponentID typeID{getUniqueComponentID()};
            return typeID;
        }

    private:
        inline static ComponentID getUniqueComponentID() {
            // states that the static ComponentID is going to start at 1
            static ComponentID lastID{1u};
            return lastID++;
        }
    };

    struct Component {};

    class IBaseComponentArray {
    public:
        virtual bool Remove(EntityID _entity) = 0;
    };

    template <typename Component, typename... Rest>
    ComponentMask CreateSignature() {
        ComponentMask signature;
        ComponentID id = ComponentIDHelper::GetID<Component>();
        signature.set(id);
        if constexpr (sizeof...(Rest) > 0) {
            signature |= CreateSignature<Rest...>();
        }
        return signature;
    }

    template <typename T>
    class ComponentArray : public IBaseComponentArray {
    private:
        /* data */
    public:
        ComponentArray();
        ~ComponentArray();
        T& Get(EntityID _entity);
        void Insert(EntityID _entity, T _component);
        void Remove(EntityID _entity, T _component);
        bool Has(EntityID _entity);
        virtual bool Remove(EntityID _entity) override;

    protected:
        Vector<T> mPackedComponents;
        Queue<int> mFreePackedIx;
        UMap<EntityID, int> mSparseIndexMap;
        UMap<int, EntityID> mDenseToSparseKey;
    };

    class ComponentArrayPool {
    public:
        template <typename T>
        inline ComponentArray<T>* GetComponentArray() {
            ComponentID id = ComponentIDHelper::GetID<T>();
            return static_cast<ComponentArray<T>*>(mComponentArrayMap.at(id));
        }

        inline IBaseComponentArray* GetComponentArray(ComponentID _componentID) {
            if (mComponentArrayMap.find(_componentID) == mComponentArrayMap.end()) {
                return nullptr;
            }
            return mComponentArrayMap.at(_componentID);
        }

        template <typename T>
        inline void AddComponentArray(ComponentArray<T>* _array) {
            ComponentID id = ComponentIDHelper::GetID<T>();
            mComponentArrayMap.insert_or_assign(id, _array);
        }

        template <typename T>
        inline void RemoveComponentArray() {
            ComponentID id = ComponentIDHelper::GetID<T>();
            mComponentArrayMap.erase(id);
        }

        inline void RemoveComponentArray(ComponentID _id) {
            mComponentArrayMap.erase(_id);
            // mComponentArrays.remove(_id);
        }

        template <typename T>
        inline bool HasComponentArray() {
            ComponentID id = ComponentIDHelper::GetID<T>();
            return mComponentArrayMap.find(id) != mComponentArrayMap.end();
        }

        inline bool HasComponentArray(ComponentID _id) {
            // return false;
            return mComponentArrayMap.find(_id) != mComponentArrayMap.end();
        }

    protected:
        UMap<ComponentID, IBaseComponentArray*> mComponentArrayMap;
    };

    class ComponentManager {};
} // namespace Umbra

#include "Component.inl"
