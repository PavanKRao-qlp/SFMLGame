
namespace Umbra {

    inline const EntityID World::CreateEntity() {
        return mWorldRegister->CreateEntity();
    }

    inline void World::DestroyEntity(EntityID _entity) {
        mWorldRegister->DestroyEntity(_entity);
    }

    inline void World::AddTag(EntityID _entity, const String& _tag) {
        mWorldRegister->AddTag(_entity, _tag);
    }

    inline bool World::IsTag(EntityID _entity, String _tag) {
        return mWorldRegister->IsTag(_entity, _tag);
    }

    inline void World::RemoveTag(EntityID _entity) {
        mWorldRegister->RemoveTag(_entity);
    }

    inline Vector<EntityID> World::FindEntitiesByTag(const String& _tag) {
        return mWorldRegister->FindEntitiesByTag(_tag);
    }

    inline const Set<EntityID>& World::GetEntitiesByTag(const String& _tag) {
        return mWorldRegister->GetEntitiesByTag(_tag);
    }

    inline bool World::HasEntitiesWithTag(const String& _tag) {
        return mWorldRegister->HasEntitiesWithTag(_tag);
    }

    inline void World::AddSystem(ESystemPhase _phase, int _priority, SharedPtr<System> _system) {
        mWorldRegister->AddSystem(_phase, _priority, _system);
    }

    inline void World::RemoveSystem(SharedPtr<System>& _system) {
        mWorldRegister->RemoveSystem(_system);
    }

    template <typename T>
    inline void World::RegisterComponent() {
        mWorldRegister->RegisterComponent<T>();
    }

    template <typename T>
    inline void World::AddComponent(EntityID _entity, T _component) {
        mWorldRegister->AddComponent<T>(_entity, _component);
    }

    template <typename T, typename... Args>
    inline void World::AddComponent(EntityID _entity, Args&&... args) {
        mWorldRegister->AddComponent<T>(_entity, std::forward<Args>(args)...);
    }

    template <typename T>
    inline void World::RemoveComponent(EntityID _entity) {
        mWorldRegister->RemoveComponent<T>(_entity);
    }

    template <typename T>
    inline bool World::HasComponent(EntityID _entity) {
        return mWorldRegister->HasComponent<T>(_entity);
    }

    template <typename T>
    inline T* World::GetComponent(EntityID _entity) {
        return mWorldRegister->GetComponent<T>(_entity);
    }


} // namespace Umbra
