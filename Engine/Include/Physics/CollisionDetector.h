#pragma once
#include "ECS/Components/BoundingVolume.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/PhysicsBodyComponent.h"
#include "ECS/Components/Transform.h"
#include "ECS/Enity.h"
#include "Math/Polygon.h"
#include "Physics/Collision.h"
#include "Physics/IBroadphaseResolver.h"
#include "Umbra.h"

namespace Umbra {
    class CollisionDetector {
    public:
        CollisionDetector();

        /**
         * @brief Tells Broadphase Resolver to clear bounding volume spatial data
         */
        void ClearBoundingVolumeSpatialData();

        /**
         * @brief Provides Broadphase Resolver Bounding Volume
         */
        void AddBoundingVolume(BoundingVolumeAABB _boundingVolume);

        /**
         * @brief Checks if a bounding volume contains point
         * @param _point Point To test
         */
        bool BroadphaseQueryColliderAt(Math::Vector2f _point);

        /**
         * @brief Checks if any bounding volume intersects AABB
         * @param _bound AABB To test
         * @param _bCompletelyInside if true then will only consider bounding volume fully inside the AABB @param
         * __bound
         */

        bool BroadphaseQueryCollidersInsideAABB(Math::Bounds2D _bound, bool _bCompletelyInside = false);
        /**
         * @brief Checks if any bounding volume intersects the given ray
         * @param _ray Ray To test
         */
        bool BroadphaseRayCast(Math::Ray2D _ray);

        Vector<Tuple<EntityID, EntityID>> RunBroadPhase();
        Vector<Collision> RunNarrowPhase(Vector<Tuple<EntityID, EntityID>>& PossibleCollisions);
        class BaseView* mBaseView;

        bool CheckPolygonPolygonOverlapSAT(Math::Polygon& _shapeA, Math::Polygon& _shapeB, Collision& _collision);
        int Clip(const Math::Vector2f& _normal, Pair<Math::Vector2f, Math::Vector2f>& _edge, float _clippingPlaneProj);

    private:
        Vector<Math::Vector2f> GetContactPointsNaive(Math::Polygon& _shapeA, Math::Polygon& _shapeB);
        /**
         *
         */
        Vector<ContactPoint> GetContactPointsViaClipping(
            Math::Polygon& _shapeA, Math::Polygon& _shapeB, Math::Vector2f& _normal);
        bool CheckCollision(EntityID _entityA, EntityID _entityB, Collision& _collision);
        bool CheckCircleCircleOverlap(Math::Vector2f _positionA, Math::Vector2f _positionB, float _radiusA,
            float _radiusB, Collision& _collision);
        bool CheckBoxBoxOverlapAABB(Math::Bounds2D _boundsA, Math::Bounds2D _boundsB, Collision& _collision);
        /**
         * @brief Broadphase Resolver implementation
         */
        SharedPtr<IBroadphaseResolver> mBroadphaseResolver;
    };

} // namespace Umbra
