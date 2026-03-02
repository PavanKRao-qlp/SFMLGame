#include "Service/Physics/CollisionQuery.h"

#include "Math/Bounds.h"
#include "Math/GeometryUtils.h"
#include "Math/MathUtils.h"

namespace Umbra::CollisionQuery {


    bool CheckCollision(const PhysicsBodyData& _a, const PhysicsBodyData& _b, CollisionDef& _collisionDef) {
        // PhysicsBodyData::Angle is in radians, but geometry functions (GetRotated,
        // GetClosestPointOnOrientedBoundEdge) expect degrees. Convert once here.
        float angleADeg = Math::RadianToDegree(_a.Angle);
        float angleBDeg = Math::RadianToDegree(_b.Angle);

        if (_a.BodyShape.IsCircle() && _b.BodyShape.IsCircle()
            && TestCircleCircle(_a.Position, _a.BodyShape.GetCircle().GetRadius(), _b.Position,
                _b.BodyShape.GetCircle().GetRadius(), _collisionDef)) {
            return true;
        }

        if (_a.BodyShape.IsCircle() && _b.BodyShape.IsBox()
            && TestCircleVsOBB(_a.Position, _a.BodyShape.GetCircle().GetRadius(), _b.Position,
                _b.BodyShape.GetBox().GetSize(), angleBDeg, _collisionDef)) {
            // TestCircleVsOBB returns normal from OBB(B) toward Circle(A).
            // Flip to maintain A->B convention.
            _collisionDef.contactNormal = -1 * _collisionDef.contactNormal;
            return true;
        }

        if (_a.BodyShape.IsBox() && _b.BodyShape.IsCircle()
            && TestCircleVsOBB(_b.Position, _b.BodyShape.GetCircle().GetRadius(), _a.Position,
                _a.BodyShape.GetBox().GetSize(), angleADeg, _collisionDef)) {
            // TestCircleVsOBB returns normal from OBB(A) toward Circle(B).
            // Already points A->B, no flip needed.
            return true;
        }
        if (_a.BodyShape.IsBox() && _b.BodyShape.IsBox()
            && TestBoxVsBoxSAT(_a.Position, _a.BodyShape.GetBox().GetSize(), angleADeg, _b.Position,
                _b.BodyShape.GetBox().GetSize(), angleBDeg, _collisionDef)) {
            return true;
        }
        return false;
    }

    bool TestCircleCircle(
        Math::Vector2f _posA, float _radiusA, Math::Vector2f _posB, float _radiusB, CollisionDef& _collisionDef) {
        Math::Vector2f displacement = _posB - _posA;
        float distSq                = displacement.SquareMagnitude();
        float radiiSum              = _radiusA + _radiusB;

        if (distSq > radiiSum * radiiSum) {
            return false;
        }

        float dist = std::sqrt(distSq);

        // Penetration is the overlap of the two radii along the contact normal
        _collisionDef.penetration = radiiSum - dist;

        // Contact normal points from A toward B along the line connecting centers.
        // If centers coincide (dist ~ 0), pick an arbitrary axis to avoid division by zero.
        if (dist > Math::EPSILON) {
            _collisionDef.contactNormal = displacement / dist;
        } else {
            _collisionDef.contactNormal = Math::Vector2f(1.0f, 0.0f);
        }

        // Contact point lies on the surface of A toward B (midpoint of overlap region)
        ContactDef contact;


        contact.contactPoint = _posA + (_collisionDef.contactNormal * _radiusA);
        //_posA + _collisionDef.contactNormal * (_radiusA - _collisionDef.penetration * 0.5f);
        contact.penetration = _collisionDef.penetration;
        _collisionDef.contacts.emplace_back(contact);

        return true;
    }


    bool TestCircleVsOBB(Math::Vector2f _posA, float _radiusA, Math::Vector2f _posB, Math::Vector2f _sizeB,
        float _angleB, CollisionDef& _collisionDef) {
        // Find the closest point on the OBB edge to the circle center.
        // If the distance from that point to the circle center is less than the radius, they overlap.
        Umbra::Math::Vector2f pointOnOBB =
            Umbra::Math::GetClosestPointOnOrientedBoundEdge(Math::Bounds2D(_posB, _sizeB), _angleB, _posA);

        Math::Vector2f displacement = _posA - pointOnOBB;
        float distance              = displacement.Magnitude();

        if (distance > _radiusA) {
            return false;
        }

        // Contact normal points from OBB surface toward the circle center.
        // NOTE: CheckCollision may flip this to maintain A->B convention.
        if (distance > Math::EPSILON) {
            _collisionDef.contactNormal = displacement / distance;
        } else {
            _collisionDef.contactNormal = Math::Vector2f(1.0f, 0.0f);
        }

        // Penetration is how far the circle sinks past the OBB edge
        _collisionDef.penetration = _radiusA - distance;

        // Contact point is the closest point on the OBB surface
        ContactDef contact;
        contact.contactPoint = pointOnOBB;
        contact.penetration  = _collisionDef.penetration;
        _collisionDef.contacts.emplace_back(contact);

        return true;
    }


    bool TestBoxVsBoxSAT(Math::Vector2f _posA, Math::Vector2f _sizeA, float _angleA, Math::Vector2f _posB,
        Math::Vector2f _sizeB, float _angleB, CollisionDef& _collisionDef) {
        Vector<Math::Vector2f> VerticesA;
        VerticesA.emplace_back(_posA + Math::Vector2f(_sizeA.x * 0.5f, -_sizeA.y * 0.5f).GetRotated(_angleA));
        VerticesA.emplace_back(_posA + Math::Vector2f(_sizeA.x * 0.5f, _sizeA.y * 0.5f).GetRotated(_angleA));
        VerticesA.emplace_back(_posA + Math::Vector2f(-_sizeA.x * 0.5f, _sizeA.y * 0.5f).GetRotated(_angleA));
        VerticesA.emplace_back(_posA + Math::Vector2f(-_sizeA.x * 0.5f, -_sizeA.y * 0.5f).GetRotated(_angleA));
        Math::Polygon polygonA = Math::Polygon(VerticesA);
        Vector<Math::Vector2f> VerticesB;
        VerticesB.emplace_back(_posB + Math::Vector2f(_sizeB.x * 0.5f, -_sizeB.y * 0.5f).GetRotated(_angleB));
        VerticesB.emplace_back(_posB + Math::Vector2f(_sizeB.x * 0.5f, _sizeB.y * 0.5f).GetRotated(_angleB));
        VerticesB.emplace_back(_posB + Math::Vector2f(-_sizeB.x * 0.5f, _sizeB.y * 0.5f).GetRotated(_angleB));
        VerticesB.emplace_back(_posB + Math::Vector2f(-_sizeB.x * 0.5f, -_sizeB.y * 0.5f).GetRotated(_angleB));
        Math::Polygon polygonB = Math::Polygon(VerticesB);
        return TestPolygonVsPolygonOverlapSAT(polygonA, polygonB, _collisionDef);
    }

    bool TestPolygonVsPolygonOverlapSAT(Math::Polygon& _shapeA, Math::Polygon& _shapeB, CollisionDef& _collisionDef) {
        Vector<Math::Vector2f> normalsA = _shapeA.GetNormals();
        Vector<Math::Vector2f> normalsB = _shapeB.GetNormals();
        Vector<Math::Vector2f> axes;
        axes.insert(axes.end(), normalsA.begin(), normalsA.end());
        axes.insert(axes.end(), normalsB.begin(), normalsB.end());
        float minOverLap = fInf;
        Math::Vector2f minAxis;

        // SAT (Separating Axis Theorem):
        // Project both polygons onto each candidate axis (edge normals of both shapes).
        // If any axis shows a gap between the projections, the shapes are separated (early out).
        // Otherwise, track the axis with the smallest overlap — this gives the
        // Minimum Translation Vector (MTV) direction and penetration depth.
        for (Math::Vector2f axis : axes) {
            Math::Polygon::Projection projectionA = _shapeA.GetProjectionOntoAxis(axis);
            Math::Polygon::Projection projectionB = _shapeB.GetProjectionOntoAxis(axis);

            if (projectionA.Min > projectionB.Max || projectionA.Max < projectionB.Min) {
                // Found a separating axis — no collision
                return false;
            }

            float overlap = Math::Min(projectionA.Max, projectionB.Max) - Math::Max(projectionA.Min, projectionB.Min);
            if (overlap < minOverLap) {
                minOverLap = overlap;
                minAxis    = axis;
            }
        }
        // inverse if axis is from b to a
        auto AB    = _shapeB.GetCenter() - _shapeA.GetCenter();
        float bInv = (Math::Vector2f::Dot(AB, minAxis));
        if (bInv < 0) {
            minAxis = -1 * minAxis;
        }

        // minAxis is the axis of least overlap (Minimum Translation Vector direction).
        // It has been flipped above to always point from A toward B.
        _collisionDef.contactNormal = minAxis;
        _collisionDef.penetration   = minOverLap;

        // Use Sutherland-Hodgman clipping on the best edges to find contact points
        _collisionDef.contacts = GetContactPointsViaClipping(_shapeA, _shapeB, _collisionDef.contactNormal);

        return true;
    }

    Vector<ContactDef> Umbra::CollisionQuery::GetContactPointsViaClipping(
        Math::Polygon& _shapeA, Math::Polygon& _shapeB, Math::Vector2f& _normal) {
        Vector<ContactDef> contacts;

        int vaIx                         = -1;
        float vaProj                     = -fInf;
        Vector<Math::Vector2f> verticesA = _shapeA.GetVertices();
        int nA                           = static_cast<int>(verticesA.size());
        // project all vertices onto the contact normal
        for (int i = 0; i < nA; i++) {
            float projection = Math::Vector2f::Dot(_normal, verticesA[i]);
            if (projection >= vaProj) {
                vaIx   = i;
                vaProj = projection;
            }
        }
        Math::Vector2f vertexNextA     = verticesA[(vaIx + 1) % nA];
        Math::Vector2f vertexPrevA     = verticesA[(vaIx - 1 + nA) % nA];
        Math::Vector2f vertToNextEdgeA = (vertexNextA - verticesA[vaIx]).GetNormalized();
        Math::Vector2f prevToVertEdgeA = (verticesA[vaIx] - vertexPrevA).GetNormalized();
        float prevProj                 = Math::Vector2f::Dot(prevToVertEdgeA, _normal);
        float nextProj                 = Math::Vector2f::Dot(vertToNextEdgeA, _normal);
        Pair<Math::Vector2f, Math::Vector2f> bestEdgeA;
        if (Math::Abs(prevProj) <= Math::Abs(nextProj)) {
            bestEdgeA = {vertexPrevA, verticesA[vaIx]};
        } else {
            bestEdgeA = {verticesA[vaIx], vertexNextA};
        }

        int vbIx                         = -1;
        float vbProj                     = -fInf;
        Vector<Math::Vector2f> verticesB = _shapeB.GetVertices();
        int nB                           = static_cast<int>(verticesB.size());
        for (int i = 0; i < nB; i++) {
            float projection = Math::Vector2f::Dot(-1 * _normal, verticesB[i]);
            if (projection >= vbProj) {
                vbIx   = i;
                vbProj = projection;
            }
        }
        // find the edge that is perpendicular to contact normal
        Math::Vector2f vertexNextB     = verticesB[(vbIx + 1) % nB];
        Math::Vector2f vertexPrevB     = verticesB[(vbIx - 1 + nB) % nB];
        Math::Vector2f vertToNextEdgeB = (vertexNextB - verticesB[vbIx]).GetNormalized();
        Math::Vector2f prevToVertEdgeB = (verticesB[vbIx] - vertexPrevB).GetNormalized();
        prevProj                       = Math::Vector2f::Dot(prevToVertEdgeB, -1 * _normal);
        nextProj                       = Math::Vector2f::Dot(vertToNextEdgeB, -1 * _normal);
        Pair<Math::Vector2f, Math::Vector2f> bestEdgeB;
        if (Math::Abs(prevProj) <= Math::Abs(nextProj)) {
            bestEdgeB = {vertexPrevB, verticesB[vbIx]};
        } else {
            bestEdgeB = {verticesB[vbIx], vertexNextB};
        }

        Pair<Math::Vector2f, Math::Vector2f> referenceEdge;
        Pair<Math::Vector2f, Math::Vector2f> incidentEdge;
        float e1Dot = Math::Abs(Math::Vector2f::Dot((bestEdgeA.second - bestEdgeA.first), _normal));
        float e2Dot = Math::Abs(Math::Vector2f::Dot((bestEdgeB.second - bestEdgeB.first), -1 * _normal));

        bool bFlipInc = false;
        if (e1Dot <= e2Dot) {
            bFlipInc      = false;
            referenceEdge = bestEdgeA;
            incidentEdge  = bestEdgeB;
        } else {
            referenceEdge = bestEdgeB;
            incidentEdge  = bestEdgeA;
            bFlipInc      = true;
        }

        Math::Vector2f refEdge   = (referenceEdge.second - referenceEdge.first).GetNormalized();
        Math::Vector2f refNormal = Math::Vector2f(refEdge.y, -refEdge.x);
        if (Math::Vector2f::Dot(_normal, refNormal) < 0) {
            refNormal = -1 * refNormal; // Flip the normal to make sure it points in the correct direction
        }

        float refC1 = Math::Vector2f::Dot(refEdge, referenceEdge.first);
        float refC2 = Math::Vector2f::Dot(-1 * refEdge, referenceEdge.second);

        Pair<Math::Vector2f, Math::Vector2f> clipped = incidentEdge;
        if (!Clip(refEdge, clipped, refC1)) {
            return contacts;
        }
        if (!Clip(refEdge * -1, clipped, refC2)) {
            return contacts;
        }
        if (bFlipInc) {
            refNormal *= -1;
        }
        float refDepth = Umbra::Math::Vector2f::Dot(refNormal, referenceEdge.first);
        for (auto& point : {clipped.first, clipped.second}) {
            float depth = Math::Vector2f::Dot(refNormal, point) - refDepth;
            if (depth <= 0.0f) {
                ContactDef contact;
                contact.contactPoint = point;
                contact.penetration  = -depth;
                contacts.emplace_back(contact);
            }
        }
        return contacts;
    }

    int Umbra::CollisionQuery::Clip(
        const Math::Vector2f& _normal, Pair<Math::Vector2f, Math::Vector2f>& _edge, float _clippingPlaneProj) {
        Vector<Math::Vector2f> result;
        float dist1 = Math::Vector2f::Dot(_normal, _edge.first) - _clippingPlaneProj;
        float dist2 = Math::Vector2f::Dot(_normal, _edge.second) - _clippingPlaneProj;

        // If points are inside, keep them
        if (dist1 >= 0.0f) {
            result.emplace_back(_edge.first);
        }
        if (dist2 >= 0.0f) {
            result.emplace_back(_edge.second);
        }

        // If the points are on different sides, clip the edge
        if (dist1 * dist2 < 0.0f) {
            float t                 = dist1 / (dist1 - dist2);
            Math::Vector2f newPoint = _edge.first + t * (_edge.second - _edge.first);
            result.push_back(newPoint);
        }

        if (result.size() < 2) {
            return 0;
        }

        _edge.first  = result[0];
        _edge.second = result[1];
        return 1;
    }

    // ============== CCD Time-of-Impact Functions ==============

    float TimeOfImpactCircleCircle(Math::Vector2f _posA0, Math::Vector2f _posA1, float _radiusA,
        Math::Vector2f _posB, float _radiusB) {
        // Sweep circle A from _posA0 to _posA1 against stationary circle B at _posB
        // pA(t) = _posA0 + t * d, where d = _posA1 - _posA0
        // Solve: |pA(t) - _posB|^2 = (rA + rB)^2
        Math::Vector2f d    = _posA1 - _posA0;
        Math::Vector2f f    = _posA0 - _posB;
        float radiiSum      = _radiusA + _radiusB;

        float a = Math::Vector2f::Dot(d, d);
        float b = 2.0f * Math::Vector2f::Dot(f, d);
        float c = Math::Vector2f::Dot(f, f) - radiiSum * radiiSum;

        // Already overlapping at t=0
        if (c <= 0.0f) {
            return 0.0f;
        }

        // No relative motion
        if (a < Math::EPSILON) {
            return 1.0f;
        }

        float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f) {
            return 1.0f; // No intersection
        }

        float sqrtDisc = Math::Sqrt(discriminant);
        float t        = (-b - sqrtDisc) / (2.0f * a);

        if (t >= 0.0f && t <= 1.0f) {
            return t;
        }
        return 1.0f;
    }

    float TimeOfImpactCircleOBB(Math::Vector2f _posA0, Math::Vector2f _posA1, float _radiusA,
        Math::Vector2f _posB, Math::Vector2f _sizeB, float _angleB) {
        // Transform sweep into OBB-local space, expand OBB by circle radius, ray-cast center
        float angleRad = Math::DegreeToRadian(_angleB);
        float cosA     = Math::Cos(-angleRad);
        float sinA     = Math::Sin(-angleRad);

        // Transform circle positions into box-local space
        Math::Vector2f d0    = _posA0 - _posB;
        Math::Vector2f d1    = _posA1 - _posB;
        Math::Vector2f localP0(d0.x * cosA - d0.y * sinA, d0.x * sinA + d0.y * cosA);
        Math::Vector2f localP1(d1.x * cosA - d1.y * sinA, d1.x * sinA + d1.y * cosA);

        // Ray from localP0 to localP1 in box-local space
        Math::Vector2f localDir = localP1 - localP0;

        // Expanded half-extents (box expanded by circle radius on each face)
        Math::Vector2f halfSize = _sizeB * 0.5f + Math::Vector2f(_radiusA, _radiusA);

        // Slab-method ray-box intersection
        float tMin = 0.0f;
        float tMax = 1.0f;

        // X slab
        if (Math::Abs(localDir.x) < Math::EPSILON) {
            if (localP0.x < -halfSize.x || localP0.x > halfSize.x) {
                return 1.0f;
            }
        } else {
            float invDx = 1.0f / localDir.x;
            float t1    = (-halfSize.x - localP0.x) * invDx;
            float t2    = (halfSize.x - localP0.x) * invDx;
            if (t1 > t2) {
                std::swap(t1, t2);
            }
            tMin = Math::Max(tMin, t1);
            tMax = Math::Min(tMax, t2);
            if (tMin > tMax) {
                return 1.0f;
            }
        }

        // Y slab
        if (Math::Abs(localDir.y) < Math::EPSILON) {
            if (localP0.y < -halfSize.y || localP0.y > halfSize.y) {
                return 1.0f;
            }
        } else {
            float invDy = 1.0f / localDir.y;
            float t1    = (-halfSize.y - localP0.y) * invDy;
            float t2    = (halfSize.y - localP0.y) * invDy;
            if (t1 > t2) {
                std::swap(t1, t2);
            }
            tMin = Math::Max(tMin, t1);
            tMax = Math::Min(tMax, t2);
            if (tMin > tMax) {
                return 1.0f;
            }
        }

        // Check if already overlapping at t=0
        if (Math::Abs(localP0.x) <= halfSize.x && Math::Abs(localP0.y) <= halfSize.y) {
            return 0.0f;
        }

        if (tMin >= 0.0f && tMin <= 1.0f) {
            return tMin;
        }
        return 1.0f;
    }

    float TimeOfImpactBoxBox(Math::Vector2f _posA0, Math::Vector2f _posA1, Math::Vector2f _sizeA, float _angleA,
        Math::Vector2f _posB, Math::Vector2f _sizeB, float _angleB, int _bisectionIterations) {
        // Binary search over t in [0,1]: interpolate position of box A, test overlap with SAT
        float lo = 0.0f;
        float hi = 1.0f;

        // First check if there's overlap at t=1 (final position)
        CollisionDef tempDef;
        if (!TestBoxVsBoxSAT(_posA1, _sizeA, _angleA, _posB, _sizeB, _angleB, tempDef)) {
            // Check a few intermediate samples to see if collision occurs during sweep
            bool bFoundCollision = false;
            for (int i = 1; i <= 4; ++i) {
                float t              = static_cast<float>(i) / 4.0f;
                Math::Vector2f posAt = _posA0 + (_posA1 - _posA0) * t;
                CollisionDef sampleDef;
                if (TestBoxVsBoxSAT(posAt, _sizeA, _angleA, _posB, _sizeB, _angleB, sampleDef)) {
                    bFoundCollision = true;
                    hi              = t;
                    break;
                }
            }
            if (!bFoundCollision) {
                return 1.0f;
            }
        }

        // Check overlap at t=0
        CollisionDef startDef;
        if (TestBoxVsBoxSAT(_posA0, _sizeA, _angleA, _posB, _sizeB, _angleB, startDef)) {
            return 0.0f;
        }

        // Binary search for earliest TOI
        for (int i = 0; i < _bisectionIterations; ++i) {
            float mid            = (lo + hi) * 0.5f;
            Math::Vector2f posMid = _posA0 + (_posA1 - _posA0) * mid;
            CollisionDef midDef;
            if (TestBoxVsBoxSAT(posMid, _sizeA, _angleA, _posB, _sizeB, _angleB, midDef)) {
                hi = mid; // Collision at mid, search earlier
            } else {
                lo = mid; // No collision at mid, search later
            }
        }

        return hi;
    }

    float ComputeTimeOfImpact(const PhysicsBodyData& _bodyA, Math::Vector2f _oldPosA,
        const PhysicsBodyData& _bodyB, int _bisectionIterations) {
        float angleBDeg = Math::RadianToDegree(_bodyB.Angle);
        float angleADeg = Math::RadianToDegree(_bodyA.Angle);

        if (_bodyA.BodyShape.IsCircle() && _bodyB.BodyShape.IsCircle()) {
            return TimeOfImpactCircleCircle(_oldPosA, _bodyA.Position,
                _bodyA.BodyShape.GetCircle().GetRadius(),
                _bodyB.Position, _bodyB.BodyShape.GetCircle().GetRadius());
        }

        if (_bodyA.BodyShape.IsCircle() && _bodyB.BodyShape.IsBox()) {
            return TimeOfImpactCircleOBB(_oldPosA, _bodyA.Position,
                _bodyA.BodyShape.GetCircle().GetRadius(),
                _bodyB.Position, _bodyB.BodyShape.GetBox().GetSize(), angleBDeg);
        }

        if (_bodyA.BodyShape.IsBox() && _bodyB.BodyShape.IsCircle()) {
            // Sweep box A vs circle B: use circle-OBB with swapped roles
            // Treat circle B as stationary, sweep box A as expanded circle check
            // For simplicity, use bisection with SAT-like overlap test
            // Actually, we can approximate: treat box A center sweeping against expanded box around circle
            // Use bisection approach for this case too
            float radiusB = _bodyB.BodyShape.GetCircle().GetRadius();
            // Expand a virtual box around the circle for bisection
            Math::Vector2f circleAsBox(radiusB * 2.0f, radiusB * 2.0f);
            return TimeOfImpactBoxBox(_oldPosA, _bodyA.Position,
                _bodyA.BodyShape.GetBox().GetSize(), angleADeg,
                _bodyB.Position, circleAsBox, 0.0f, _bisectionIterations);
        }

        if (_bodyA.BodyShape.IsBox() && _bodyB.BodyShape.IsBox()) {
            return TimeOfImpactBoxBox(_oldPosA, _bodyA.Position,
                _bodyA.BodyShape.GetBox().GetSize(), angleADeg,
                _bodyB.Position, _bodyB.BodyShape.GetBox().GetSize(), angleBDeg,
                _bisectionIterations);
        }

        return 1.0f;
    }

} // namespace Umbra::CollisionQuery
