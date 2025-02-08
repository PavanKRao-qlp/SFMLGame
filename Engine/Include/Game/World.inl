
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

    inline void World::AddSystem(SharedPtr<System> _system) {
        mWorldRegister->AddSystem(_system);
    }

    inline void World::RemoveSystem(SharedPtr<System>& _system) {
        mWorldRegister->RemoveSystem(_system);
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
        mWorldRegister->RegisterComponent<T>(_entity);
    }

    template <typename T>
    inline bool World::HasComponent(EntityID _entity) {
        return mWorldRegister->HasComponent<T>(_entity);
    }

    template <typename T>
    inline T* World::GetComponent(EntityID _entity) {
        return return mWorldRegister->GetComponent<T>(_entity);
    }


} // namespace Umbra
