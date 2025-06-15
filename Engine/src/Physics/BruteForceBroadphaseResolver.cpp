#include "Physics/BruteForceBroadphaseResolver.h"
namespace Umbra {
    void BruteForceBroadphaseResolver::ClearBoundingVolumeSpatialData() {
        mBoundingVolumeArray.clear();
    }

    void BruteForceBroadphaseResolver::AddBoundingVolume(BoundingVolumeAABB _boundingVolume) {
        mBoundingVolumeArray.emplace_back(_boundingVolume);
    }

    bool BruteForceBroadphaseResolver::QueryColliderAt(Math::Vector2f _point) {
        bool bFound = false;
        for (BoundingVolumeAABB& volumeAABB : mBoundingVolumeArray) {
            if (volumeAABB.SlimBounds.Contains(_point)) {
                bFound = true;
            }
        }
        return bFound;
    }

    bool BruteForceBroadphaseResolver::QueryCollidersInsideAABB(Math::Bounds2D _bound, bool _bCompletelyInside) {
        Vector<BoundingVolumeAABB> volumesInsideQueryBound;
        for (BoundingVolumeAABB& volumeAABB : mBoundingVolumeArray) {
            if (_bCompletelyInside) {
                if (_bound.Contains(volumeAABB.SlimBounds)) {
                    volumesInsideQueryBound.push_back(volumeAABB);
                }
            } else {
                if (_bound.Intersects(volumeAABB.SlimBounds)) {
                    volumesInsideQueryBound.push_back(volumeAABB);
                }
            }
        }
        if (volumesInsideQueryBound.size() > 0) {
            return true;
        }
        return false;
    }

    bool BruteForceBroadphaseResolver::RayCast(Math::Ray2D _ray) {
        Vector<BoundingVolumeAABB> volumesHit;
        for (BoundingVolumeAABB& volumeAABB : mBoundingVolumeArray) {
            // using slab method
            float txMin = (volumeAABB.SlimBounds.Min().x - _ray.Position.x) / _ray.Direction.x;
            float txMax = (volumeAABB.SlimBounds.Max().x - _ray.Position.x) / _ray.Direction.x;
            if (txMin > txMax) {
                float minTemp = txMin;
                txMin         = txMax;
                txMax         = minTemp;
            }
            float tyMin = (volumeAABB.SlimBounds.Min().y - _ray.Position.y) / _ray.Direction.y;
            float tyMax = (volumeAABB.SlimBounds.Max().y - _ray.Position.y) / _ray.Direction.y;
            if (tyMin > tyMin) {
                float minTemp = tyMin;
                tyMin         = tyMin;
                tyMin         = minTemp;
            }
            if ((txMin > tyMax) || (tyMin > txMax)) {
                continue;
            } else if (Math::Max(txMin, tyMin) >= 0 || volumeAABB.SlimBounds.Contains(_ray.Position)) {
                volumesHit.push_back(volumeAABB);
            }

            if (volumesHit.size() > 0) {
                return true;
            }
        }
        return false;
    }

    void BruteForceBroadphaseResolver::DetectBroadphaseCollisionpair() {
        Vector<Tuple<EntityID, EntityID>> collisionPairs;
        for (int i = 0; i < mBoundingVolumeArray.size(); i++) {
            for (int j = 0; j < mBoundingVolumeArray.size(); j++) {
                        }
        }
    }

} // namespace Umbra
