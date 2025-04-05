#pragma once
#include "ECS/Component.h"
#include "Math/Bounds.h"
#include "Math/Vector.h"
namespace Umbra {
    class ColliderComponent {
    public:
        Math::Vector2f Offset;
    };

    class BoxColliderComponent : public ColliderComponent {
    public:
        Math::Bounds2D Bounds;
    };


    class CircleColliderComponent : public ColliderComponent {
    public:
        float Radius;
    };
} // namespace Umbra
