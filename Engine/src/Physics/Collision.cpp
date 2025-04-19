#include "Physics/Collision.h"

#include "ECS/ECSRegister.h"
#include "ECS/Systems/RenderSystem.h"
#include "Umbra.h"

namespace Umbra {
    void ContactResolver::ResolveContacts(Vector<Collision>& _collisions, double _deltaTime) {
        for (Collision collision : _collisions) {
            std::sort(collision.GetContacts().begin(), collision.GetContacts().end(),
                [](const ContactPoint& a, const ContactPoint& b) {
                    // Sort by penetration depth (deepest first)
                    // Priority 1: Most penetrating contacts
                    return (a.mPenetration > b.mPenetration);
                });
        }
        std::sort(_collisions.begin(), _collisions.end(), [](Collision& a, Collision& b) {
            // Sort by penetration depth (deepest first)
            // Priority 1: Most penetrating contacts
            return (a.GetContacts()[0].mPenetration > b.GetContacts()[0].mPenetration);
        });

        for (int iteration = 0; iteration < mMaxIteration; iteration++) {
            for (Collision collision : _collisions) {
                for (ContactPoint& contact : collision.GetContacts()) {

                    RenderSystem::DebugDrawLine(contact.mContactPosition,
                        contact.mContactPosition + (contact.mContactNormal * contact.mPenetration), sf::Color::Red);
                    ResolveContact(&contact, collision.mBodyA, collision.mBodyB, _deltaTime);
                }
            }
        }
    }


    void ContactResolver::ResolveContact(
        ContactPoint* _contact, EntityID _entityA, EntityID _entityB, double _deltaTime) {
        PhysicsBodyComponent* bodyA    = mBaseView->ecsRegister->GetComponent<PhysicsBodyComponent>(_entityA);
        PhysicsBodyComponent* bodyB    = mBaseView->ecsRegister->GetComponent<PhysicsBodyComponent>(_entityB);
        TransformComponent* transformA = mBaseView->ecsRegister->GetComponent<TransformComponent>(_entityA);
        TransformComponent* transformB = mBaseView->ecsRegister->GetComponent<TransformComponent>(_entityB);
        CalculateSeparatingVelocity(_contact, bodyA, bodyB, _deltaTime);
        ResolvePenetration(_contact, bodyA, transformA, bodyB, transformB);
    }

    void ContactResolver::CalculateSeparatingVelocity(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
        PhysicsBodyComponent* _physicsBodyB, double _deltaTime) {

        /*
             velocity of object in the direction of the contact.
             if  < 0 object are moving towards each other
        */
        double separatingVelocity =
            Math::Vector2f::Dot((_physicsBodyA->mVelocity - _physicsBodyB->mVelocity), _contact->mContactNormal);
        if (separatingVelocity > 0) {
            // bodies already moving away from each other or are stationary
            // return;
        }
        // Calculate acceleration-induced velocity
        double velocityCausedViaAcceleration =
            Math::Vector2f::Dot((_physicsBodyA->mAcceleration - _physicsBodyB->mAcceleration), _contact->mContactNormal)
            * _deltaTime;


        // velocity required to stop further penetration + bounce
        // Remove acceleration-caused closing velocity
        double desiredDeltaVelocity = 0;
        double cofOfRestitution = 1; // Math::Min(_physicsBodyA->mCofOfRestitution, _physicsBodyB->mCofOfRestitution);
        if (velocityCausedViaAcceleration < 0) {
            // Case 1: Acceleration is pushing objects together
            desiredDeltaVelocity =
                -separatingVelocity * (1 + cofOfRestitution) + (velocityCausedViaAcceleration * cofOfRestitution);
            desiredDeltaVelocity = Math::Max(desiredDeltaVelocity, 0.0);
        } else {
            // Case 2: Acceleration isn't contributing to collision
            desiredDeltaVelocity = -separatingVelocity * (1 + cofOfRestitution);
        }

        double totalInverseMass = _physicsBodyA->mInverseMass + _physicsBodyB->mInverseMass;
        if (totalInverseMass <= 0) {
            // All bodies are static cannot apply impulse
            return;
        }
        // An impulse is the rate of change of the momentum, i.e. a force delivered in an instant
        //  impulse is force * Delta time
        //  since force = ma and acceleration is dv/dt
        //  impulse = m*dv ie dv/inverseMass
        //  impulse per unit of inverse mass.
        float impulseMagnitude = (float) (desiredDeltaVelocity / totalInverseMass);
        // impulse as vector
        Math::Vector2f impulsePerUnitInverseMass = _contact->mContactNormal * impulseMagnitude;
        _physicsBodyA->mVelocity += _physicsBodyA->mInverseMass * impulsePerUnitInverseMass;
        _physicsBodyB->mVelocity -= _physicsBodyB->mInverseMass * impulsePerUnitInverseMass;
    }

    void ContactResolver::ResolvePenetration(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
        TransformComponent* _transformA, PhysicsBodyComponent* _physicsBodyB, TransformComponent* _transformB) {


        const float POSITION_SLOP = 0.01f; // Small allowed penetration

        if (_contact->mPenetration <= POSITION_SLOP) {
            //  we don’t have any penetration
            return;
        }
        double totalInverseMass = _physicsBodyA->mInverseMass + _physicsBodyB->mInverseMass;
        if (totalInverseMass <= 0) {
            // All bodies are static cannot move
            return;
        }
        Math::Vector2f displacementPerUnitInverseMass =
            _contact->mContactNormal * (_contact->mPenetration / totalInverseMass) * -1;
        _transformA->Position += displacementPerUnitInverseMass * _physicsBodyA->mInverseMass;
        _transformB->Position -= displacementPerUnitInverseMass * _physicsBodyB->mInverseMass;
    }

    Vector<Tuple<EntityID, EntityID>> Umbra::CollisionDetector::RunBroadPhase() {
        return Vector<Tuple<EntityID, EntityID>>();
    }

    Vector<Collision> CollisionDetector::RunNarrowPhase(Vector<Tuple<EntityID, EntityID>>& PossibleCollisions) {
        Vector<Collision> collisions;

        for (auto itA = mBaseView->mEntities.begin(); itA != mBaseView->mEntities.end(); ++itA) {
            EntityID entityA = *itA;
            for (auto itB = std::next(itA); itB != mBaseView->mEntities.end(); ++itB) {
                EntityID entityB = *itB;
                Collision collision;
                if (CheckCollision(entityA, entityB, collision)) {
                    collisions.emplace_back(collision);
                }
            }
        }
        return collisions;
    }


    bool CollisionDetector::CheckPolygonPolygonOverlapSAT(
        Math::Polygon& _shapeA, Math::Polygon& _shapeB, Collision& _collision) {
        Vector<Math::Vector2f> normalsA = _shapeA.GetNormals();
        Vector<Math::Vector2f> normalsB = _shapeB.GetNormals();
        Vector<Math::Vector2f> axes;
        axes.insert(axes.end(), normalsA.begin(), normalsA.end());
        axes.insert(axes.end(), normalsB.begin(), normalsB.end());
        float minOverLap = fInf;
        Math::Vector2f minAxis;
        // For every axis project both shape and find if any axis exist which has no overlap
        // if overlap is not found objects are separated
        // else find the axis with minimum overlap to find minimum translation vector
        int i = 0;
        int v = 0;
        for (Math::Vector2f axis : axes) {
            i++;
            Math::Polygon::Projection projectionA = _shapeA.GetProjectionOntoAxis(axis);
            Math::Polygon::Projection projectionB = _shapeB.GetProjectionOntoAxis(axis);
            if (projectionA.Min > projectionB.Max || projectionA.Max < projectionB.Min) {
                // axis is the separating axis theorem
                return false;
            }
            float overlap = Math::Min(projectionA.Max, projectionB.Max) - Math::Max(projectionA.Min, projectionB.Min);
            if (overlap < minOverLap) {
                minOverLap = overlap;
                minAxis    = axis;
                v          = i;
            }
        }
        // inverse if axis is from b to a
        auto AB    = _shapeB.GetCenter() - _shapeA.GetCenter();
        float bInv = (Math::Vector2f::Dot(AB, minAxis));
        if (bInv < 0) {
            minAxis = -1 * minAxis;
        }

        _collision.mContactNormal = minAxis;
        _collision.AddContacts(GetContactPointsViaClipping(_shapeA, _shapeB, _collision.mContactNormal));
        return true;
    }

    int CollisionDetector::Clip(
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

    Vector<ContactPoint> Umbra::CollisionDetector::GetContactPointsViaClipping(
        Math::Polygon& _shapeA, Math::Polygon& _shapeB, Math::Vector2f& _normal) {
        Vector<ContactPoint> collisionPoint;

        int vaIx                         = -1;
        float vaProj                     = -fInf;
        Vector<Math::Vector2f> verticesA = _shapeA.GetVertices();
        for (int i = 0; i < verticesA.size(); i++) {
            float projection = Math::Vector2f::Dot(_normal, verticesA[i]);
            if (projection >= vaProj) {
                vaIx   = i;
                vaProj = projection;
            }
        }
        Math::Vector2f vertexNextA     = verticesA[(vaIx + 1) % verticesA.size()];
        Math::Vector2f vertexPrevA     = verticesA[(vaIx - 1) % verticesA.size()];
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
        for (int i = 0; i < verticesB.size(); i++) {
            float projection = Math::Vector2f::Dot(-1 * _normal, verticesB[i]);
            if (projection >= vbProj) {
                vbIx   = i;
                vbProj = projection;
            }
        }
        // find the edge that is perpendicular to contact normal
        Math::Vector2f vertexNextB     = verticesB[(vbIx + 1) % verticesB.size()];
        Math::Vector2f vertexPrevB     = verticesB[(vbIx - 1) % verticesB.size()];
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

        float refC1 = Math::Vector2f::Dot(refNormal, referenceEdge.first);
        float refC2 = Math::Vector2f::Dot(-1 * refNormal, referenceEdge.second);

        Pair<Math::Vector2f, Math::Vector2f> clipped = incidentEdge;
        if (!Clip(-1 * refNormal, clipped, refC1)) {
            return collisionPoint;
        }
        if (!Clip(refNormal, clipped, refC2)) {
            return collisionPoint;
        }

        float refDepth = Math::Vector2f::Dot(_normal, referenceEdge.first);
        for (auto& point : {incidentEdge.first, incidentEdge.second}) {
            float depth = Math::Vector2f::Dot(_normal, point) - refDepth;
            if (depth <= 0.0f) {
                ContactPoint cp;
                cp.mContactPosition = point;
                cp.mPenetration     = -depth;
                cp.mContactNormal   = bFlipInc ? -1 * _normal : _normal;
                collisionPoint.emplace_back(cp);
            }
        }
        return collisionPoint;
    }

    Vector<Math::Vector2f> CollisionDetector::GetContactPointsNaive(Math::Polygon& _shapeA, Math::Polygon& _shapeB) {
        Vector<Math::Vector2f> points;
        for (Math::Vector2f vertex : _shapeA.GetVertices()) {
            bool bInside = true;

            for (Math::Vector2f normal : _shapeB.GetNormals()) {
                Math::Polygon::Projection projectionB = _shapeB.GetProjectionOntoAxis(normal);
                float projectionVert                  = Math::Vector2f::Dot(vertex, normal);

                if (projectionVert < projectionB.Min || projectionVert > projectionB.Max) {
                    bInside = false;
                    break;
                }
            }
            if (bInside) {
                points.emplace_back(vertex);
            }
        }
        for (Math::Vector2f vertex : _shapeB.GetVertices()) {
            bool bInside = true;

            for (Math::Vector2f normal : _shapeA.GetNormals()) {
                Math::Polygon::Projection projectionB = _shapeA.GetProjectionOntoAxis(normal);
                float projectionVert                  = Math::Vector2f::Dot(vertex, normal);

                if (projectionVert < projectionB.Min || projectionVert > projectionB.Max) {
                    bInside = false;
                    break;
                }
            }
            if (bInside) {
                points.emplace_back(vertex);
            }
        }
        return points;
    }

    bool CollisionDetector::CheckCollision(EntityID _entityA, EntityID _entityB, Collision& _collision) {
        bool bCollision                    = false;
        TransformComponent* transformA     = mBaseView->ecsRegister->GetComponent<TransformComponent>(_entityA);
        PhysicsBodyComponent* physicsBodyA = mBaseView->ecsRegister->GetComponent<PhysicsBodyComponent>(_entityA);
        TransformComponent* transformB     = mBaseView->ecsRegister->GetComponent<TransformComponent>(_entityB);
        PhysicsBodyComponent* physicsBodyB = mBaseView->ecsRegister->GetComponent<PhysicsBodyComponent>(_entityB);
        if (mBaseView->ecsRegister->HasComponent<CircleColliderComponent>(_entityA)
            && mBaseView->ecsRegister->HasComponent<CircleColliderComponent>(_entityB)) {
            CircleColliderComponent* circleColliderA =
                mBaseView->ecsRegister->GetComponent<CircleColliderComponent>(_entityA);
            CircleColliderComponent* circleColliderB =
                mBaseView->ecsRegister->GetComponent<CircleColliderComponent>(_entityB);
            if (CheckCircleCircleOverlap(transformA->Position + circleColliderA->Offset,
                    transformB->Position + circleColliderB->Offset, circleColliderA->Radius, circleColliderB->Radius,
                    _collision)) {
                bCollision = true;
            }
        } else if (mBaseView->ecsRegister->HasComponent<BoxColliderComponent>(_entityA)
                   && mBaseView->ecsRegister->HasComponent<BoxColliderComponent>(_entityB)) {
            BoxColliderComponent* boxColliderA = mBaseView->ecsRegister->GetComponent<BoxColliderComponent>(_entityA);
            BoxColliderComponent* boxColliderB = mBaseView->ecsRegister->GetComponent<BoxColliderComponent>(_entityB);
            if (transformA->Angle == 0 && transformB->Angle == 0) {
                Math::Bounds2D boundsA(transformA->Position + boxColliderA->Offset, boxColliderA->Size);
                Math::Bounds2D boundsB(transformB->Position + boxColliderB->Offset, boxColliderB->Size);
                if (CheckBoxBoxOverlapAABB(boundsA, boundsB, _collision)) {
                    bCollision = true;
                }
            } else {
            }
        }
        if (bCollision) {
            _collision.mBodyA            = _entityA;
            _collision.mBodyB            = _entityB;
            _collision.mCofOfRestitution = Math::Min(physicsBodyA->mCofOfRestitution, physicsBodyB->mCofOfRestitution);
        }

        return bCollision;
    }


    bool CollisionDetector::CheckCircleCircleOverlap(
        Math::Vector2f _positionA, Math::Vector2f _positionB, float _radiusA, float _radiusB, Collision& _collision) {

        RenderSystem::DebugDrawCircle(_positionA, _radiusA);
        RenderSystem::DebugDrawCircle(_positionB, _radiusB);

        // Check if circles are overlapping
        Math::Vector2f displacement = _positionB - _positionA;
        float radiiSum              = _radiusA + _radiusB;
        if (displacement.SquareMagnitude() <= Math::Pow(radiiSum, 2)) {
            ContactPoint contact;
            float distance = displacement.Magnitude();
            if (distance > 0) {
                contact.mContactNormal = displacement.GetNormalized();
            } else {
                // handle when object overlaps 100% give arbitrary direction
                contact.mContactNormal = Math::Vector2f(1, 0);
            }
            contact.mPenetration     = (_radiusA + _radiusB) - distance;
            contact.mContactPosition = _positionA + (contact.mContactNormal * _radiusA);
            //_positionA + ((_radiusA - contact.mPenetration) * contact.mContactNormal);
            _collision.AddContact(contact);

            RenderSystem::DebugDrawCircle(contact.mContactPosition, 1, true, sf::Color::Red);
            RenderSystem::DebugDrawLine(contact.mContactPosition,
                contact.mContactPosition + (-1 * contact.mContactNormal * contact.mPenetration));

            return true;
        }
        return false;
    }

    bool CollisionDetector::CheckBoxBoxOverlapAABB(
        Math::Bounds2D _boundsA, Math::Bounds2D _boundsB, Collision& _collision) {
        RenderSystem::DrawDebugBox(_boundsA);
        RenderSystem::DrawDebugBox(_boundsB);
        if (_boundsA.Intersects(_boundsB)) {
        }
        return false;
    }

    void Collision::AddContact(ContactPoint& _contact) {
        mContacts.emplace_back(_contact);
    }
    void Collision::AddContacts(Vector<ContactPoint>& _contacts) {
        mContacts.insert(mContacts.end(), _contacts.begin(), _contacts.end());
    }

    Vector<ContactPoint>& Collision::GetContacts() {
        return mContacts;
    }
} // namespace Umbra
