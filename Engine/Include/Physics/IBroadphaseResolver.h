#pragma once
#include "ECS/Components/BoundingVolume.h"
#include "Math/Bounds.h"
#include "Math/Ray.h"
#include "Math/Vector.h"
#include "Physics/Collision.h"
#include "Umbra.h"

namespace Umbra {
    class IBroadphaseResolver {
    public:
        /**
         * @brief Removes All bounding volume from internal storage data structure
         */
        virtual void ClearBoundingVolumeSpatialData() = 0;

        /** @brief Adds a bounding volume to internal storage data structure*/
        virtual void AddBoundingVolume(BoundingVolumeAABB _boundingVolume) = 0;

        /**
         * @brief Checks if a bounding volume contains point
         * @param _point Point To test
         */
        virtual bool QueryColliderAt(Math::Vector2f _point) = 0;

        /**
         * @brief Checks if any bounding volume intersects AABB
         * @param _bound AABB To test
         * @param _bCompletelyInside if true then will only consider bounding volume fully inside the AABB @param
         * __bound
         */
        virtual bool QueryCollidersInsideAABB(Math::Bounds2D _bound, bool _bCompletelyInside = false) = 0;

        /**
         * @brief Checks if any bounding volume intersects the given ray
         * @param _ray Ray To test
         */
        virtual bool RayCast(Math::Ray2D _ray) = 0;

        /**
         * @brief returns pair of colliding bounding volumes
         */
        virtual void DetectBroadphaseCollisionpair() = 0;
    };
} // namespace Umbra
