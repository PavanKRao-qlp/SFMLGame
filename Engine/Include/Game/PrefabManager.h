#pragma once
#include "Core/Singleton.h"
#include "Game/EntityTemplate.h"
#include "Umbra.h"

namespace Umbra {
    class World;

    /**
     * Global registry of named entity prefab factories.
     *
     * Usage:
     *   // Register once in IGameInstance::Initialize()
     *   PrefabManager::GetInstance()->Register("Bullet", [tex](World& w) {
     *       EntityID e = w.CreateEntity();
     *       w.AddComponent<TransformComponent>(e, ...);
     *       w.AddComponent<SpriteComponent>(e, tex);
     *       return e;
     *   });
     *
     *   // Spawn from any scene
     *   EntityID bullet = world->Instantiate("Bullet");
     *
     *   // Spawn with position override
     *   EntityID bullet = world->Instantiate("Bullet", [&](EntityID _e) {
     *       world->GetComponent<TransformComponent>(_e)->Position = spawnPos;
     *   });
     */
    class PrefabManager : public Singleton<PrefabManager> {
    public:
        // Register a named prefab factory. Overwrites any existing registration with a warning.
        void Register(const String& _name, EntityTemplate _factory);

        // Returns true if a prefab with the given name has been registered.
        bool Has(const String& _name) const;

        // Instantiate a prefab by name in the given world.
        // Returns MAX_ENTITY and logs a warning if the name is not registered.
        EntityID Instantiate(const String& _name, World& _world) const;

        // Instantiate a prefab by name, then call _overrideFn on the new entity.
        // The override runs before the entity is returned to the caller.
        EntityID Instantiate(const String& _name, World& _world,
                             std::function<void(EntityID)> _overrideFn) const;

        // Remove all registered prefabs.
        void Clear();

    private:
        friend class Singleton<PrefabManager>;
        PrefabManager() = default;

        UMap<String, EntityTemplate> mRegistry;
    };

} // namespace Umbra
