#pragma once
#include "ECS/Component.h"
#include "Math/Vector.h"
namespace Umbra {
    struct RigidBodyComponent : Component {
    public:
        inline RigidBodyComponent() {}
        Math::Vector2f Velocity;
        /* data */
    };
} // namespace Umbra
