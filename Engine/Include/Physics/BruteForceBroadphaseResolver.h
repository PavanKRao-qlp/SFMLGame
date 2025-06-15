#pragma once
#include "ECS/Components/BoundingVolume.h"
#include "Physics/IBroadphaseResolver.h"
#include "Umbra.h"

namespace Umbra {
    class BruteForceBroadphaseResolver : public IBroadphaseResolver {
    public:
        virtual void ClearBoundingVolumeSpatialData() override;
        virtual void AddBoundingVolume(BoundingVolumeAABB _boundingVolume) override;
        virtual bool QueryColliderAt(Math::Vector2f _point) override;
        virtual bool QueryCollidersInsideAABB(Math::Bounds2D _bound, bool _bCompletelyInside = false) override;
        virtual bool RayCast(Math::Ray2D _ray) override;
        virtual void DetectBroadphaseCollisionpair() override;

    private:
        Vector<BoundingVolumeAABB> mBoundingVolumeArray;
    };
} // namespace Umbra
