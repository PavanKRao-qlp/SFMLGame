#include "Game/PrefabManager.h"
#include "Diag/Logger.h"

namespace Umbra {

    void PrefabManager::Register(const String& _name, EntityTemplate _factory) {
        if (mRegistry.count(_name)) {
            UMBRA_LOG_WARN("PrefabManager: overwriting existing prefab '%s'", _name.c_str());
        }
        mRegistry[_name] = std::move(_factory);
        UMBRA_LOG_INFO("PrefabManager: registered prefab '%s'", _name.c_str());
    }

    bool PrefabManager::Has(const String& _name) const {
        return mRegistry.count(_name) > 0;
    }

    EntityID PrefabManager::Instantiate(const String& _name, World& _world) const {
        auto it = mRegistry.find(_name);
        if (it == mRegistry.end()) {
            UMBRA_LOG_WARN("PrefabManager: prefab '%s' not found", _name.c_str());
            return MAX_ENTITY;
        }
        return it->second(_world);
    }

    EntityID PrefabManager::Instantiate(const String& _name, World& _world,
                                        std::function<void(EntityID)> _overrideFn) const {
        EntityID entity = Instantiate(_name, _world);
        if (entity != MAX_ENTITY && _overrideFn) {
            _overrideFn(entity);
        }
        return entity;
    }

    void PrefabManager::Clear() {
        mRegistry.clear();
    }

} // namespace Umbra
