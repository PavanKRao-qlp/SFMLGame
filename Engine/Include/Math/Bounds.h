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

        /// @brief Tests if a ray intersects this AABB using the slab method
        /// @param _origin Ray origin
        /// @param _invDirection Inverse of ray direction (1/dir for each component)
        /// @param _maxDistance Maximum ray distance
        /// @return True if the ray intersects within [0, _maxDistance]
        bool RayIntersects(const Vector2f& _origin, const Vector2f& _invDirection, float _maxDistance) const {
            Vector2f bMin = Min();
            Vector2f bMax = Max();

            float t1 = (bMin.x - _origin.x) * _invDirection.x;
            float t2 = (bMax.x - _origin.x) * _invDirection.x;
            float t3 = (bMin.y - _origin.y) * _invDirection.y;
            float t4 = (bMax.y - _origin.y) * _invDirection.y;

            float tMin = std::max(std::min(t1, t2), std::min(t3, t4));
            float tMax = std::min(std::max(t1, t2), std::max(t3, t4));

            if (tMax < 0.0f || tMin > tMax) {
                return false;
            }
            // The nearest intersection is tMin if >= 0, else tMax (origin inside box)
            float tHit = tMin >= 0.0f ? tMin : tMax;
            return tHit <= _maxDistance;
        }

        Vector2f Center;
        Vector2f Size;
    };

} // namespace Umbra::Math
