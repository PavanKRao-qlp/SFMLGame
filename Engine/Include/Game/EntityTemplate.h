#pragma once
#include "ECS/ECSConfig.h"
#include "EnginePCH.h"

namespace Umbra {
    class World;

    // A prefab factory: given a World, constructs a new entity and returns its ID.
    using EntityTemplate = std::function<EntityID(World&)>;

} // namespace Umbra
