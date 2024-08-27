#include "ECSRegister.h"

namespace D2D
{
    inline ECSRegister::ECSRegister(/* args */)
    {
        ComponentArrayHolder = new ComponentArrayPool();
    };

    inline ECSRegister::~ECSRegister() {};

    inline EntityID ECSRegister::CreateEntity()
    {
        if (nextEntityId > MAX_ENTITY)
        {
            // handle
            return MAX_ENTITY;
        }
        Entity *e = new Entity(nextEntityId++);
        e->bAlive = true;
        mEntityComponentSignatures[e->GetId()] = 0u;
        mEntities.emplace(e->GetId(), e);
        bRegisterDirty = true;
        return e->GetId();
    };

    inline EntityID ECSRegister::DestroyEntity() {

    };

    template <typename T>
    inline void ECSRegister::RegisterComponent()
    {
        ComponentID id = ComponentIDHelper::GetID<T>();
        // add contains check
        ComponentArrayHolder->AddComponentArray(new ComponentArray<T>());
    };

    template <typename T>
    inline void ECSRegister::AddComponent(EntityID _entity, T _component)
    {
        ComponentID id = ComponentIDHelper::GetID<T>();
        Entity *e = mEntities[_entity];
        mEntityComponentSignatures[_entity].set(id,true);
        ComponentArray<T> *pool = ComponentArrayHolder->GetComponentArray<T>();
        pool->Insert(_entity, _component);
        // pool->mComponents.insert(_component);
    };

    template <typename T, typename... Args>
    void ECSRegister::AddComponent(EntityID _entity, Args &&...args)
    {
        T component(std::forward<Args>(args)...);
        AddComponent<T>(_entity, component);
    }

    template <typename T>
    inline bool ECSRegister::HasComponent(EntityID _entity)
    {
        ComponentID id = ComponentIDHelper::GetID<T>();
        return mEntityComponentSignatures[_entity].test(id);
    };

    template <typename T>
    inline T *ECSRegister::GetComponent(EntityID _entity)
    {
        if (!HasComponent<T>(_entity))
            return nullptr;
        ComponentID id = ComponentIDHelper::GetID<T>();
        ComponentArray<T> *pool = ComponentArrayHolder->GetComponentArray<T>();
        return &pool->Get(_entity);
    };

    inline void ECSRegister::AddSystem(System *_system)
    {
        mSystems.emplace_back(_system);
        _system->AssignRegistry(this);
    }

    inline void ECSRegister::Update()
    {

        // handle creation / destruction of entities

        // handle component pools

        // update system component pool
        if (bRegisterDirty)
        {
            for (System *system : mSystems)
            {
                for (auto pair : mEntities) // has to be sparse set
                {
                    EntityID entity = pair.first;
                    if ((system->SystemSignature & mEntityComponentSignatures[entity]) == system->SystemSignature)
                    {
                        system->AddEntity(entity);
                    }
                    else
                    {
                        system->RemoveEntity(entity);
                    }
                }
            }
            bRegisterDirty = false;
        }
        for (System *system : mSystems)
        {
            system->Update();
        }
    };
}