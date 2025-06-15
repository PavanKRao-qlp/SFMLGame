#pragma once
#include "EnginePCH.h"
#include "Math/MathUtils.h"
#include "Math/Vector.h"

namespace Umbra::Math {

    class Bounds2D {
    public:
        Bounds2D() {}
        Bounds2D(const Vector2f& _center, const Vector2f& _size) : Center(_center), Size(_size) {}
        bool Contains(const Vector2f& _point) const;
        bool Contains(const Bounds2D& _bound) const;
        bool Intersects(const Bounds2D& _bound) const;
        void Encapsulate(const Vector2f& _point);
        Vector2f Extents() const;
        Vector2f Min() const;
        Vector2f Max() const;

        Vector2f Center;
        Vector2f Size;
    };

} // namespace Umbra::Math
