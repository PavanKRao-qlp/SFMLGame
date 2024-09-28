#include "ECSRegister.h"

namespace Umbra {
    inline ECSRegister::ECSRegister(/* args */) {
        ComponentArrayHolder = new ComponentArrayPool();
    }

    inline ECSRegister::~ECSRegister() {};

    inline EntityID ECSRegister::CreateEntity() {
        EntityID id                    = mEntityManager.CreateEntity();
        mEntityComponentSignatures[id] = ComponentMask();
        bRegisterDirty                 = true;
        return id;
    }

    inline void ECSRegister::DestroyEntity(EntityID _entity) {
        mEntityManager.DestroyEntity(_entity);
        bRegisterDirty = true;
    }

    template <typename T>
    inline void ECSRegister::RegisterComponent() {
        ComponentID id = ComponentIDHelper::GetID<T>();
        if (!ComponentArrayHolder->HasComponentArray<T>()) {
            ComponentArrayHolder->AddComponentArray(new ComponentArray<T>());
        }
    }

    template <typename T>
    inline void ECSRegister::AddComponent(EntityID _entity, T _component) {
        if (!mEntityManager.IsValid(_entity)) {
            return;
        }
        if (!ComponentArrayHolder->HasComponentArray<T>()) {
            return;
        }
        ComponentID id = ComponentIDHelper::GetID<T>();
        mEntityComponentSignatures[_entity].set(id, true);
        ComponentArray<T>* pool = ComponentArrayHolder->GetComponentArray<T>();
        pool->Insert(_entity, _component);
    }

    template <typename T, typename... Args>
    void ECSRegister::AddComponent(EntityID _entity, Args&&... args) {
        T component(std::forward<Args>(args)...);
        AddComponent<T>(_entity, component);
    }

    template <typename T>
    inline void ECSRegister::RemoveComponent(EntityID _entity) {
        if (!mEntityManager.IsValid(_entity)) {
            return;
        }
        if (!ComponentArrayHolder->HasComponentArray<T>()) {
            return;
        }
        ComponentID id = ComponentIDHelper::GetID<T>();
        mEntityComponentSignatures[_entity].set(id, false);
        ComponentArray<T>* pool = ComponentArrayHolder->GetComponentArray<T>();
        pool->Remove(_entity, _component);
    }

    template <typename T>
    inline bool ECSRegister::HasComponent(EntityID _entity) {
        ComponentID id = ComponentIDHelper::GetID<T>();
        return mEntityComponentSignatures[_entity].test(id);
    }

    template <typename T>
    inline T* ECSRegister::GetComponent(EntityID _entity) {
        if (!mEntityManager.IsValid(_entity)) {
            return nullptr;
        }
        if (!ComponentArrayHolder->HasComponentArray<T>()) {
            return nullptr;
        }
        ComponentID id          = ComponentIDHelper::GetID<T>();
        ComponentArray<T>* pool = ComponentArrayHolder->GetComponentArray<T>();
        return &(pool->Get(_entity));
    }

    inline void ECSRegister::AddSystem(System* _system) {
        mSystems.emplace_back(_system);
        _system->AssignRegistry(this);
    }

    inline void ECSRegister::Update() {
        if (bRegisterDirty) {
            for (EntityID entity : mEntityManager.EntitiesRemoved) {
                // remove entity from mEntities
                mEntityManager.Entities.erase(entity);
                for (ComponentID cId = 0; cId < MAX_COMPONENTS; ++cId) {
                    if (mEntityComponentSignatures[entity].test(cId)) {
                        // ComponentArrayHolder->GetComponentArray(cId);
                    }
                }
                mEntityComponentSignatures[entity].reset();
                // remove component pool
                // remove from systems
                for (System* system : mSystems) {
                    system->RemoveEntity(entity);
                }
            }
            mEntityManager.EntitiesRemoved.clear();

            for (EntityID entity : mEntityManager.EntitiesAdded) {
                mEntityManager.Entities.emplace(entity);
            }
            mEntityManager.EntitiesAdded.clear();

            for (System* system : mSystems) {
                for (EntityID entity : mEntityManager.Entities) // has to be sparse set
                {
                    if ((system->SystemSignature & mEntityComponentSignatures[entity]) == system->SystemSignature) {
                        system->AddEntity(entity);
                    } else {
                        system->RemoveEntity(entity);
                    }
                }
            }
            bRegisterDirty = false;
        }
        for (System* system : mSystems) {
            if (system->GetEnabled()) {
                system->Update();
            }
        }
    }
} // namespace Umbra
