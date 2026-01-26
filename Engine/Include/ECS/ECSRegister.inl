#include "Diag/Logger.h"
#include "ECSRegister.h"

namespace Umbra {
    inline ECSRegister::ECSRegister(/* args */) {
        mComponentManager = new ComponentManager();
        RegisterComponent<TagComponent>();
    }

    inline ECSRegister::~ECSRegister() {
        delete mComponentManager;
        mComponentManager = nullptr;
    }

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
        // for (auto system : mSystems) {
        //     system->Flush();
        // }
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

        // Mark entity as modified if it's already active (not newly created)
        if (mEntityManager.Entities.find(_entity) != mEntityManager.Entities.end()) {
            mEntityManager.EntitiesModified.emplace(_entity);
            bRegisterDirty = true;
        }
    }

    inline void ECSRegister::AddTag(EntityID _entity, const String& _tag) {
        if (!mEntityManager.IsValid(_entity)) {
            return;
        }
        if (HasComponent<TagComponent>(_entity)) {
            TagComponent* tagComponent = GetComponent<TagComponent>(_entity);
            // Remove from old tag index if tag is changing
            if (tagComponent->Tag != _tag) {
                auto it = mTagIndex.find(tagComponent->Tag);
                if (it != mTagIndex.end()) {
                    it->second.erase(_entity);
                    if (it->second.empty()) {
                        mTagIndex.erase(it);
                    }
                }
            }
            tagComponent->Tag = _tag;
        } else {
            AddComponent<TagComponent>(_entity, _tag);
        }
        // Add to new tag index
        mTagIndex[_tag].insert(_entity);
    }

    inline void ECSRegister::RemoveTag(EntityID _entity) {
        if (!mEntityManager.IsValid(_entity)) {
            return;
        }
        if (!HasComponent<TagComponent>(_entity)) {
            return;
        }
        TagComponent* tagComponent = GetComponent<TagComponent>(_entity);
        // Remove from tag index
        auto it = mTagIndex.find(tagComponent->Tag);
        if (it != mTagIndex.end()) {
            it->second.erase(_entity);
            if (it->second.empty()) {
                mTagIndex.erase(it);
            }
        }
        RemoveComponent<TagComponent>(_entity);
    }

    inline Vector<EntityID> ECSRegister::FindEntitiesByTag(const String& _tag) {
        auto it = mTagIndex.find(_tag);
        if (it != mTagIndex.end()) {
            return Vector<EntityID>(it->second.begin(), it->second.end());
        }
        return Vector<EntityID>();
    }

    inline const Set<EntityID>& ECSRegister::GetEntitiesByTag(const String& _tag) {
        auto it = mTagIndex.find(_tag);
        if (it != mTagIndex.end()) {
            return it->second;
        }
        return mEmptyEntitySet;
    }

    inline bool ECSRegister::HasEntitiesWithTag(const String& _tag) {
        auto it = mTagIndex.find(_tag);
        return it != mTagIndex.end() && !it->second.empty();
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
        pool->Remove(_entity);

        // Mark entity as modified if it's active
        if (mEntityManager.Entities.find(_entity) != mEntityManager.Entities.end()) {
            mEntityManager.EntitiesModified.emplace(_entity);
            bRegisterDirty = true;
        }
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

        // Mark entity as modified if it's active
        if (mEntityManager.Entities.find(_entity) != mEntityManager.Entities.end()) {
            mEntityManager.EntitiesModified.emplace(_entity);
            bRegisterDirty = true;
        }
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
        if (!pool->Has(_entity)) {
            return nullptr;
        }
        return &(pool->Get(_entity));
    }


    inline void ECSRegister::AddSystem(ESystemPhase _systemPhase, int _priority, SharedPtr<System> _system) {
        if (mSystemMap.find(_systemPhase) == mSystemMap.end()) {
            mSystemMap.emplace(_systemPhase, Vector<SharedPtr<System>>());
        }
        _system->SetPriority(_priority);
        mSystemMap[_systemPhase].emplace_back(_system);
        _system->AssignRegistry(this);

        // Sort systems by priority (lower priority values execute first)
        // Using stable_sort to preserve insertion order for systems with equal priority
        std::stable_sort(mSystemMap[_systemPhase].begin(), mSystemMap[_systemPhase].end(),
            [](const SharedPtr<System>& _a, const SharedPtr<System>& _b) {
                return _a->GetPriority() < _b->GetPriority();
            });
    }

    inline void ECSRegister::RemoveSystem(SharedPtr<System>& _system) {
        // for (auto it = mSystems.begin(); it != mSystems.end(); ++it) {
        //     if (*it == _system) {
        //         mSystems.erase(it);
        //         break; // Stop after removing the first match
        //     }
        // }
    }

    inline void ECSRegister::Update() {
        CleanUp();
        for (const auto& pair : mSystemMap) {
            for (const SharedPtr<System>& system : pair.second) {
                if (system->GetEnabled()) {
                    system->Update();
                }
            }
        }
    }

    inline void ECSRegister::Update(ESystemPhase _systemPhase) {
        for (const SharedPtr<System>& system : mSystemMap[_systemPhase]) {
            if (system->GetEnabled()) {
                system->Update();
            }
        }
    }

    inline void ECSRegister::CleanUp() {
        if (bRegisterDirty) {
            // Remove destroyed entities from all systems first (before clearing the list)
            for (EntityID entity : mEntityManager.EntitiesDestroyed) {
                for (const auto& pair : mSystemMap) {
                    for (const SharedPtr<System>& system : pair.second) {
                        system->RemoveEntity(entity);
                    }
                }
            }
            RemoveDestroyedEntities();

            // Process only newly created entities (before clearing the list)
            for (EntityID entity : mEntityManager.EntitiesAdded) {
                mEntityManager.Entities.emplace(entity);
                for (const auto& pair : mSystemMap) {
                    for (const SharedPtr<System>& system : pair.second) {
                        if ((system->SystemSignature & mEntityComponentSignatures.at(entity))
                            == system->SystemSignature) {
                            system->AddEntity(entity);
                        }
                    }
                }
            }
            mEntityManager.EntitiesAdded.clear();

            // Process entities whose component signatures changed
            for (EntityID entity : mEntityManager.EntitiesModified) {
                for (const auto& pair : mSystemMap) {
                    for (const SharedPtr<System>& system : pair.second) {
                        if ((system->SystemSignature & mEntityComponentSignatures.at(entity))
                            == system->SystemSignature) {
                            system->AddEntity(entity);
                        } else {
                            system->RemoveEntity(entity);
                        }
                    }
                }
            }
            mEntityManager.EntitiesModified.clear();

            bRegisterDirty = false;
        }
    }

    inline void ECSRegister::RemoveDestroyedEntities() {
        for (EntityID entity : mEntityManager.EntitiesDestroyed) {
            // Clean up tag index if entity has a tag
            if (HasComponent<TagComponent>(entity)) {
                TagComponent* tagComponent = GetComponent<TagComponent>(entity);
                auto it = mTagIndex.find(tagComponent->Tag);
                if (it != mTagIndex.end()) {
                    it->second.erase(entity);
                    if (it->second.empty()) {
                        mTagIndex.erase(it);
                    }
                }
            }
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
