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

    Vector2f GetClosestPointInsideOrientedBound(Bounds2D _bounds, float _angle, Vector2f _point) {
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
    Vector2f GetClosestPointOnOrientedBoundEdge(Bounds2D _bounds, float _angle, Vector2f _point) {
        Vector2f closestPoint  = _bounds.Center;
        Vector2f centerToPoint = _point - _bounds.Center;
        Vector2f halfSize      = _bounds.Size * 0.5f;

        // X axis local basis vector rotated
        Vector2f rotatedU = Vector2f(1, 0).GetRotated(_angle);
        // Y axis local basis vector rotated
        Vector2f rotatedV = Vector2f(0, 1).GetRotated(_angle);

        float projOnU = Vector2f::Dot(rotatedU, centerToPoint);
        float projOnV = Vector2f::Dot(rotatedV, centerToPoint);
        bool bInsideU = Abs(projOnU) <= halfSize.x;
        bool bInsideV = Abs(projOnV) <= halfSize.y;
        if (bInsideU && bInsideV) {
            float distToEdgeU = halfSize.x - Abs(projOnU);
            float distToEdgeV = halfSize.y - Abs(projOnV);
            if (distToEdgeV > distToEdgeU) {
                // MTV on x axis
                projOnU = (projOnU >= 0 ? halfSize.x : -halfSize.x);
            } else {
                // MTV on y axis
                projOnV = (projOnV >= 0 ? halfSize.y : -halfSize.y);
            }
        } else {
            projOnU = Clamp(projOnU, -halfSize.x, halfSize.x);
            projOnV = Clamp(projOnV, -halfSize.y, halfSize.y);
        }
        closestPoint += rotatedU * projOnU;
        closestPoint += rotatedV * projOnV;
        return closestPoint;
    }

    bool TestRayLineSegment(Ray2D _ray, Vector2f _pointA, Vector2f _pointB, Vector2f& _outHitPoint) {
        //  line formula = P + v*d  where d = [-inf,inf]
        // ray is ro + rd * u (0-inf)
        // line is pa + (pb-pa)* v (0- 1)
        // if  2 line intersect then ro + rd * u  = pa + (pb-pa)*v
        // we find u and v and say intersection found if u >= 0 & 0 <= v >= 1
        // in matrix
        /*
            [  r_dx    -(p2x - p1x)  ] . [ u ]   =   [ p1x - r0x ]
            [  r_dy    -(p2y - p1y)  ]   [ v ]   =   [ p1y - r0y ]
         */
        // see crammer rule
        Vector2f vecAR = _pointA - _ray.Position;
        Vector2f vecAB = _pointB - _pointA;
        float denom    = Vector2f::Cross2D(_ray.Direction, vecAB);
        if (denom == 0) {
            return false; // Parallel or colinear
        }

        float u = Vector2f::Cross2D(vecAR, vecAB) / denom;
        float v = Vector2f::Cross2D(vecAR, _ray.Direction) / denom;

        if (u >= 0 && v >= 0 && v <= 1) {
            _outHitPoint = _ray.Position + _ray.Direction * u;
            return true;
        }
        return false;
    }

    bool Umbra::Math::TestRayAABB(Ray2D _ray, Bounds2D _bounds, Vector2f& _outHitPoint) {
        /** ray param formula is p = ro + (rd*t)
         *  if a point P satisfies the line formula then t = (p-ro)/rd
         *  if p is on line then (px - rox)/rdx == (py - roy)/rdy
         *  else tx will give point on ray that passes through a line/plane along y axis where x = px
         *  and ty will give point on ray that passes through a line/plane along  x axis where y = py
         **/

        // (p - ro) when p = min
        Vector2f rayToMin = (_bounds.Min() - _ray.Position);
        float tMinX       = (rayToMin.x) / (_ray.Direction.x);
        float tMinY       = (rayToMin.y) / (_ray.Direction.y);
        // (p - ro) when p = max
        Vector2f rayToMax = (_bounds.Max() - _ray.Position);
        float tMaxX       = (rayToMax.x) / (_ray.Direction.x);
        float tMaxY       = (rayToMax.y) / (_ray.Direction.y);
        /**
         * !! the division we see below can become expensive if say we wish to raycast against
         *  thousands of AABBs so optimization is we have a cached 1/rayDir in ray class and do
         *  rayToMin.x * _ray.InvDir().x
         */


        // we need to find furthest t the point nearest to ray Orig  and nearest t to point furthest from ray Orig
        // we say that the line collides with AABB when furthestNearPoint <= nearestFarPoint
        // and ray collides if furthestNearPoint > 0
        float nearestX  = Min(tMinX, tMaxX);
        float nearestY  = Min(tMinY, tMaxY);
        float furthestX = Max(tMinX, tMaxX);
        float furthestY = Max(tMinY, tMaxY);
        // ray hit entry
        float furthestNearPoint = Max(nearestX, nearestY);
        // ray hit exit
        float nearestFarPoint = Min(furthestX, furthestY);

        if (furthestNearPoint > 0 && furthestNearPoint <= nearestFarPoint) {
            _outHitPoint = _ray.Position + _ray.Direction * furthestNearPoint;
            return true;
        }
        return false;
    }

    bool TestRayOBB(Ray2D _ray, Bounds2D _bounds, float _angle, Vector2f& _outHitPoint) {
        Vector2f boxXBasis     = Vector2f(Cos(_angle), Sin(_angle));
        Vector2f boxYBasis     = Vector2f(-Sin(_angle), Cos(_angle));
        Vector2f boxToPoint    = (_ray.Position - _bounds.Center);
        Vector2f rayPosInLocal = Vector2f(Vector2f::Dot(boxToPoint, boxXBasis), Vector2f::Dot(boxToPoint, boxYBasis));
        Vector2f rayDirInLocal =
            Vector2f(Vector2f::Dot(_ray.Direction, boxXBasis), Vector2f::Dot(_ray.Direction, boxYBasis));
        // must  be in local (centered at 0) space
        Vector2f localMin = -1 * _bounds.Size * 0.5f;
        Vector2f localMax = 1 * _bounds.Size * 0.5f;
        Vector2f rayToMin = (localMin - rayPosInLocal);
        float tMinX       = (rayToMin.x) / (rayDirInLocal.x);
        float tMinY       = (rayToMin.y) / (rayDirInLocal.y);
        // (p - ro) when p = max
        Vector2f rayToMax = (localMax - rayPosInLocal);
        float tMaxX       = (rayToMax.x) / (rayDirInLocal.x);
        float tMaxY       = (rayToMax.y) / (rayDirInLocal.y);
        /**
         * !! the division we see below can become expensive if say we wish to raycast against
         *  thousands of AABBs so optimization is we have a cached 1/rayDir in ray class and do
         *  rayToMin.x * _ray.InvDir().x
         */


        // we need to find furthest t the point nearest to ray Orig  and nearest t to point furthest from ray Orig
        // we say that the line collides with AABB when furthestNearPoint <= nearestFarPoint
        // and ray collides if furthestNearPoint > 0
        float nearestX  = Min(tMinX, tMaxX);
        float nearestY  = Min(tMinY, tMaxY);
        float furthestX = Max(tMinX, tMaxX);
        float furthestY = Max(tMinY, tMaxY);
        // ray hit entry
        float furthestNearPoint = Max(nearestX, nearestY);
        // ray hit exit
        float nearestFarPoint = Min(furthestX, furthestY);

        if (furthestNearPoint > 0 && furthestNearPoint <= nearestFarPoint) {
            _outHitPoint = _ray.Position + _ray.Direction * furthestNearPoint;
            return true;
        }
        return false;
    }
} // namespace Umbra::Math
