#pragma once
#include "ECS/Component.h"
#include "EnginePCH.h"

namespace Umbra {

    /// Attached to every ECS entity spawned from an LDtk Entity layer.
    /// Game code can use this to distinguish entity types and read custom fields.
    struct LDtkEntityComponent : Component {
        String              typeName; // LDtk entity definition name (mirrors the Tag)
        UMap<String, String> fields;  // custom LDtk fields serialised to strings
    };

} // namespace Umbra
