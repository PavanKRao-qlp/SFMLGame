
#include "Math/Bounds.h"

namespace Umbra::Math {

    bool Bounds2D::Contains(const Vector2f& _point) const {
        Vector2f minPoint = Min();
        Vector2f maxPoint = Max();
        return (_point.x >= minPoint.x && _point.x <= maxPoint.x && _point.y >= minPoint.y && _point.y <= maxPoint.y);
    }

    bool Bounds2D::Intersects(const Bounds2D& _bounds) const {
        Vector2f thisMin  = Min();
        Vector2f thisMax  = Max();
        Vector2f otherMin = _bounds.Min();
        Vector2f otherMax = _bounds.Max();
        return thisMin.x <= otherMax.x && thisMax.x >= otherMin.x && thisMin.y <= otherMax.y && thisMax.y >= otherMin.y;
    }

    void Bounds2D::Encapsulate(const Vector2f& _point) {
        Vector2f minPoint = Min();
        Vector2f maxPoint = Max();
        minPoint          = Vector2f(Math::Min(minPoint.x, _point.x), Math::Min(minPoint.y, _point.y));
        maxPoint          = Vector2f(Math::Max(maxPoint.x, _point.x), Math::Max(maxPoint.y, _point.y));
        Center            = Math::Vector2f(minPoint + maxPoint) * 0.5f;
        Size              = maxPoint - minPoint;
    }

    Vector2f Bounds2D::Extents() const {
        return Size / 2.f;
    }

    Vector2f Bounds2D::Min() const {
        return Center - Extents();
    }

    Vector2f Bounds2D::Max() const {
        return Center + Extents();
    }
} // namespace Umbra::Math
