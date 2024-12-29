#pragma once
#include "ECS/Component.h"
#include "ECS/Enity.h"
#include "Math/Vector.h"
namespace Umbra {
    struct CollisionEventComponent : Component {
    public:
        inline CollisionEventComponent(EntityID _entityA, EntityID _entityB) {
            mEntityA = _entityA;
            mEntityB = _entityB;
        }
        EntityID mEntityA;
        EntityID mEntityB;
    };
} // namespace Umbra
