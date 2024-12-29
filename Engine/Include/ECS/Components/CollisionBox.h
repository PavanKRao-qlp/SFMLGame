#pragma once
#include "ECS/Component.h"
#include "Math/Bounds.h"
namespace Umbra {
    struct CollisionBoxComponent : Component {
    public:
        inline CollisionBoxComponent(Math::Bounds2D _bounds2D) {
            Bounds2D = _bounds2D;
        }
        Math::Bounds2D Bounds2D;
    };
} // namespace Umbra
