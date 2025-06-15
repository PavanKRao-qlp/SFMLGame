#pragma once
#include "ECS/Component.h"
#include "Math/Bounds.h"
#include "Math/Vector.h"
namespace Umbra {
    class ColliderComponent {

    public:
        // virtual Math::Bounds2D GetBoundingBox() = 0;
        Math::Vector2f Offset;
    };


    class BoxColliderComponent : public ColliderComponent {
    public:
        BoxColliderComponent() {}
        BoxColliderComponent(Math::Vector2f _size) : Size(_size) {}
        // inline virtual Math::Bounds2D GetBoundingBox() override {
        //     return Math::Bounds2D();
        // }
        Math::Vector2f Size;
    };


    class CircleColliderComponent : public ColliderComponent {
    public:
        // inline virtual Math::Bounds2D GetBoundingBox() override {
        //     return Math::Bounds2D();
        // }
        float Radius;
    };

    class CapsuleCollider : public ColliderComponent {
    public:
        // inline virtual Math::Bounds2D GetBoundingBox() override {
        //     return Math::Bounds2D();
        // }
    };
} // namespace Umbra
