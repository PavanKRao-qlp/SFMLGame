#pragma once
#include "Math/Bounds.h"
#include "Math/Ray.h"
#include "Math/Vector.h"
namespace Umbra::Math {

    /**
     * @brief Returns the closest point on a line segment to a given point in 2D.
     *
     * Uses vector projection to find the nearest point on the segment and clamps it within bounds.
     */
    Vector2f GetClosestPointInLineSegment(Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point);

    /**
     * @brief Returns the shortest distance from a point to a line segment.
     *
     * Uses vector projection to find the closest point and computes the distance to it.
     */
    float GetDistanceToClosestPointInLineSegment(Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point);

    /**
     * @brief Returns the shortest orthogonal distance from a point to a line segment.
     *
     * Uses vector projection to find the point one the line formed by the line segment and computes the distance to it.
     */
    float GetOrthogonalDistanceFromPointToLineSegment(Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point);

    /**
     * @brief Returns the unit normal vector pointing from the line segment to the point.
     *
     * Perpendicular direction from the closest point on the segment to the given point.
     */
    Vector2f GetOrthogonalNormalToPointForLineSegment(Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point);

    /**
     * @brief Checks if a point lies exactly on a given line segment.
     *
     * Compares the point's position using vector projection and bounding checks.
     */
    bool IsPointOnLineSegment(
        Vector2f _segmentStart, Vector2f _segmentEnd, Vector2f _point, double _tolerance = EPSILON);

    /**
     * @brief Returns the closest point to a given position that lies inside the AABB.
     *
     * The point is clamped to the bounds of the AABB. If the point is already inside, it is returned as-is.
     */
    Vector2f GetClosestPointInsideBound(Bounds2D _bounds, Vector2f _point);

    /**
     * @brief Returns the closest point on the edge of the AABB to a given position.
     *
     * If the point is inside the AABB, the closest point on the edge is returned instead.
     */
    Vector2f GetClosestPointOnBoundEdge(Bounds2D _bounds, Vector2f _point);

    /**
     * @brief Returns the closest point to a given position that lies inside the OBB.
     *
     * The point is clamped to the bounds of the OBB. If the point is already inside, it is returned as-is.
     */
    Vector2f GetClosestPointInsideOrientedBound(Bounds2D _bounds, float _angle, Vector2f _point);

    /**
     * @brief Returns the closest point on the edge of the OBB to a given position.
     *
     * If the point is inside the OBB, the closest point on the edge is returned instead.
     */
    Vector2f GetClosestPointOnOrientedBoundEdge(Bounds2D _bounds, float _angle, Vector2f _point);
    /**
     * Returns true if a 2D ray intersects a line segment; outputs the hit point if so.
     * @param _outHitPoint : sets provided vector to the result of intersection
     */
    bool TestRayLineSegment(Ray2D _ray, Vector2f _pointA, Vector2f _pointB, Vector2f& _outHitPoint);

    /**
     * Returns true if a 2D ray intersects a AABB; outputs the hit point if so.
     * @param _outHitPoint : sets provided vector to the result of intersection
     */
    bool TestRayAABB(Ray2D _ray, Bounds2D _bounds, Vector2f& _outHitPoint);

    /**
     * Returns true if a 2D ray intersects a OBB; outputs the hit point if so.
     * @param _outHitPoint : sets provided vector to the result of intersection
     */
    bool TestRayOBB(Ray2D _ray, Bounds2D _bounds, float _angle, Vector2f& _outHitPoint);
} // namespace Umbra::Math
