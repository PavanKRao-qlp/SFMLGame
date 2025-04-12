#include "Math/GeometryUtils.h"

#include "Umbra.h"


namespace Umbra::Math {
    Vector2f GetClosestPointInLineSegment(Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point) {
        Vector2f pointOnSegment;
        // Let AB be vector from _segmentStart to _segmentEnd
        Vector2f VecAB = _segmentEnd - _segmentStart;
        // Let AP be vector from _segmentStart to _point
        Vector2f vecAP = _point - _segmentStart;
        /*
         we can projection magnitude |s| = AP.AB / |AB|
         then get point by  _segmentStart + |s| * AB^
         OR we can get  projection |s| scaled by |AB| via
         t =  AP.AB / AB.AB
         and point via _segmentStart + t * AB
         since its _segmentStart + (|S|/|AB|) * AB
         we will chose the later as square root is avoided
         making it computationally faster
        */
        float scaledProjection = (Vector2f::Dot(vecAP, VecAB) / Vector2f::Dot(VecAB, VecAB));

        if (scaledProjection <= 0) {
            pointOnSegment = _segmentStart;
        } else if (scaledProjection >= 1) {
            pointOnSegment = _segmentEnd;
        } else {
            pointOnSegment = _segmentStart + (scaledProjection * VecAB);
        }
        return pointOnSegment;
    }

    Vector2f GetOrthogonalNormalToPointForLineSegment(Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point) {
        Vector2f normal;
        // Let AB be vector from _segmentStart to _segmentEnd
        Vector2f VecAB = _segmentEnd - _segmentStart;
        // Let AP be vector from _segmentStart to _point
        Vector2f vecAP = _point - _segmentStart;
        // see GetClosestPointInLineSegment
        float scaledProjection = (Vector2f::Dot(vecAP, VecAB) / Vector2f::Dot(VecAB, VecAB));

        Vector2f pointOnSegment = _segmentStart + (scaledProjection * VecAB);
        pointOnSegment          = _segmentStart + (scaledProjection * VecAB);
        normal                  = (_point - pointOnSegment);
        normal.Normalize();
        return normal;
    }

    float GetDistanceToClosestPointInLineSegment(Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point) {
        Vector2f pointOnSegment;
        // Let AB be vector from _segmentStart to _segmentEnd
        Vector2f VecAB = _segmentEnd - _segmentStart;
        // Let AP be vector from _segmentStart to _point
        Vector2f vecAP = _point - _segmentStart;
        // see GetClosestPointInLineSegment
        float scaledProjection = (Vector2f::Dot(vecAP, VecAB) / Vector2f::Dot(VecAB, VecAB));
        if (scaledProjection <= 0) {
            pointOnSegment = _segmentStart;
        } else if (scaledProjection >= 1) {
            pointOnSegment = _segmentEnd;
        } else {
            pointOnSegment = _segmentStart + (scaledProjection * VecAB);
        }
        Vector2f normal = (_point - pointOnSegment);
        return normal.Magnitude();
    }

    float GetOrthogonalDistanceFromPointToLineSegment(Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point) {
        Vector2f normal;
        // Let AB be vector from _segmentStart to _segmentEnd
        Vector2f VecAB = _segmentEnd - _segmentStart;
        // Let AP be vector from _segmentStart to _point
        Vector2f vecAP = _point - _segmentStart;
        // see GetClosestPointInLineSegment
        float scaledProjection = (Vector2f::Dot(vecAP, VecAB) / Vector2f::Dot(VecAB, VecAB));

        Vector2f pointOnSegment = _segmentStart + (scaledProjection * VecAB);
        pointOnSegment          = _segmentStart + (scaledProjection * VecAB);
        normal                  = (_point - pointOnSegment);
        return normal.Magnitude();
    }

    bool IsPointOnLineSegment(Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point, double _tolerance) {
        return GetDistanceToClosestPointInLineSegment(_segmentStart, _segmentEnd, _point) < _tolerance;
    }


    Vector2f GetClosestPointInsideBound(Bounds2D _bounds, Vector2f _point) {
        Vector2f pointInBound = _point;
        if (_point.x < _bounds.Min().x) {
            pointInBound.x = _bounds.Min().x;
        }
        if (_point.x > _bounds.Max().x) {
            pointInBound.x = _bounds.Max().x;
        }
        if (_point.y < _bounds.Min().y) {
            pointInBound.y = _bounds.Min().y;
        }
        if (_point.y > _bounds.Max().y) {
            pointInBound.y = _bounds.Max().y;
        }
        return pointInBound;
    }

    Vector2f GetClosestPointOnBoundEdge(Bounds2D _bounds, Vector2f _point) {
        // if()
        Vector2f pointOnBound  = GetClosestPointInsideBound(_bounds, _point);
        Vector2f pointToCenter = _bounds.Center - pointOnBound;
        Vector2f halfSize      = _bounds.Size * 0.5f;
        float xDispSqr         = (halfSize.x - Abs(pointToCenter.x));
        float yDispSqr         = (halfSize.y - Abs(pointToCenter.y));

        if (xDispSqr < yDispSqr) {
            if (_point.x < _bounds.Center.x) {
                pointOnBound.x = _bounds.Min().x;
            }
            if (_point.x > _bounds.Center.x) {
                pointOnBound.x = _bounds.Max().x;
            }
        } else {
            if (_point.y < _bounds.Center.y) {
                pointOnBound.y = _bounds.Min().y;
            }
            if (_point.y > _bounds.Center.y) {
                pointOnBound.y = _bounds.Max().y;
            }
        }
        return pointOnBound;
    }

    Vector2f GetClosestPointOnOrientedBound(Bounds2D _bounds, float _angle, Vector2f _point) {
        Vector2f closestPoint  = _bounds.Center;
        Vector2f centerToPoint = _point - _bounds.Center;
        Vector2f halfSize      = _bounds.Size * 0.5f;

        // X axis local basis vector rotated
        Vector2f rotatedU = Vector2f(1, 0).GetRotated(_angle);
        // Y axis local basis vector rotated
        Vector2f rotatedV = Vector2f(0, 1).GetRotated(_angle);

        float projOnU = Vector2f::Dot(rotatedU, centerToPoint);
        projOnU       = Clamp(projOnU, -halfSize.x, halfSize.x);
        closestPoint += rotatedU * projOnU;
        float projOnV = Vector2f::Dot(rotatedV, centerToPoint);
        projOnV       = Clamp(projOnV, -halfSize.y, halfSize.y);
        closestPoint += rotatedV * projOnV;
        return closestPoint;
    }

} // namespace Umbra::Math
