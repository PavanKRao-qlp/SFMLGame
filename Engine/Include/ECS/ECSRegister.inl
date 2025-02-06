#include "Diag/Logger.h"
#include "ECSRegister.h"

namespace Umbra {
    inline ECSRegister::ECSRegister(/* args */) {
        mComponentManager = new ComponentManager();
        RegisterComponent<TagComponent>();
    }

    inline ECSRegister::~ECSRegister() {}

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

    inline void ECSRegister::FlushRegister() {
        mEntityManager.Flush();
        mComponentManager->Flush();
        for (auto system : mSystems) {
            system->Flush();
        }
        mEntityComponentSignatures.clear();
    }

    template <typename T>
    inline void ECSRegister::RegisterComponent() {
        ComponentID id = ComponentIDHelper::GetID<T>();
        if (!mComponentManager->HasComponentArray<T>()) {
            mComponentManager->AddComponentArray(new ComponentArray<T>());
        }
    }

    template <typename T>
    inline void ECSRegister::AddComponent(EntityID _entity, T _component) {
        if (!mEntityManager.IsValid(_entity)) {
            return;
        }
        if (!mComponentManager->HasComponentArray<T>()) {
            return;
        }
        ComponentID id = ComponentIDHelper::GetID<T>();
        mEntityComponentSignatures.at(_entity).set(id, true);
        ComponentArray<T>* pool = mComponentManager->GetComponentArray<T>();
        pool->Insert(_entity, _component);
    }

    inline void ECSRegister::AddTag(EntityID _entity, const String& _tag) {
        if (!mEntityManager.IsValid(_entity)) {
            return;
        }
        if (HasComponent<TagComponent>(_entity)) {
            TagComponent* tagComponent = GetComponent<TagComponent>(_entity);
            tagComponent->Tag          = _tag;
        } else {
            AddComponent<TagComponent>(_entity, _tag);
        }
    }

    inline bool ECSRegister::IsTag(EntityID _entity, String _tag) {
        if (!mEntityManager.IsValid(_entity)) {
            return false;
        }
        if (!HasComponent<TagComponent>(_entity)) {
            return false;
        }
        TagComponent* tagComponent = GetComponent<TagComponent>(_entity);
        if (tagComponent != nullptr) {
            if (tagComponent->Tag == _tag) {
                return true;
            }
        }
        return false;
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
        if (!mComponentManager->HasComponentArray<T>()) {
            return;
        }
        ComponentID id = ComponentIDHelper::GetID<T>();
        mEntityComponentSignatures.at(_entity).set(id, false);
        ComponentArray<T>* pool = mComponentManager->GetComponentArray<T>();
        pool->Remove(_entity, _component);
    }

    inline void ECSRegister::RemoveComponent(EntityID _entity, ComponentID _componentId) {
        if (!mEntityManager.IsValid(_entity)) {
            return;
        }
        if (!mEntityComponentSignatures.at(_entity).test(_componentId)) {
            return;
        }
        IBaseComponentArray* pool = mComponentManager->GetComponentArray(_componentId);
        pool->Remove(_entity);
    }

    template <typename T>
    inline bool ECSRegister::HasComponent(EntityID _entity) {
        ComponentID id = ComponentIDHelper::GetID<T>();
        return mEntityComponentSignatures.at(_entity).test(id);
    }

    template <typename T>
    inline T* ECSRegister::GetComponent(EntityID _entity) {
        if (!mEntityManager.IsValid(_entity)) {
            return nullptr;
        }
        if (!mComponentManager->HasComponentArray<T>()) {
            return nullptr;
        }
        ComponentID id          = ComponentIDHelper::GetID<T>();
        ComponentArray<T>* pool = mComponentManager->GetComponentArray<T>();
        return &(pool->Get(_entity));
    }


    inline void ECSRegister::AddSystem(SharedPtr<System> _system) {
        mSystems.emplace_back(_system);
        _system->AssignRegistry(this);
    }

    inline void ECSRegister::RemoveSystem(SharedPtr<System>& _system) {
        for (auto it = mSystems.begin(); it != mSystems.end(); ++it) {
            if (*it == _system) {
                mSystems.erase(it);
                break; // Stop after removing the first match
            }
        }
    }

    inline void ECSRegister::Update() {
        if (bRegisterDirty) {
            RemoveDestroyedEntities();
            AddCreatedEntities();

            for (const SharedPtr<System>& system : mSystems) {
                for (EntityID entity : mEntityManager.Entities) // has to be sparse set
                {
                    if ((system->SystemSignature & mEntityComponentSignatures.at(entity)) == system->SystemSignature) {
                        system->AddEntity(entity);
                    } else {
                        system->RemoveEntity(entity);
                    }
                }
            }
            bRegisterDirty = false;
        }
        for (const SharedPtr<System>& system : mSystems) {
            if (system->GetEnabled()) {
                system->Update();
            }
        }
    }

    inline void ECSRegister::RemoveDestroyedEntities() {
        for (EntityID entity : mEntityManager.EntitiesDestroyed) {
            // remove entity from mEntities
            mEntityManager.RemoveEntity(entity);
            // remove component pool
            for (ComponentID cId = 0; cId < MAX_COMPONENTS; ++cId) {
                if (mEntityComponentSignatures.at(entity).test(cId)) {
                    IBaseComponentArray* pool = mComponentManager->GetComponentArray(cId);
                    if (pool != nullptr) {
                        pool->Remove(entity);
                    }
                }
            }
            mEntityComponentSignatures.at(entity).reset();
            // remove from systems
            // for (System* system : mSystems) {
            //     system->RemoveEntity(entity);
            // }
        }
        mEntityManager.EntitiesDestroyed.clear();
    }

    inline void ECSRegister::AddCreatedEntities() {
        for (EntityID entity : mEntityManager.EntitiesAdded) {
            mEntityManager.Entities.emplace(entity);
        }
        mEntityManager.EntitiesAdded.clear();
    }
} // namespace Umbra
