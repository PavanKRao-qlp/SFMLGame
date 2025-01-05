
#include "Math/CollisionSystem.h"

#include "ECS/Systems/CollisionEventResolverSystem.h"


namespace Umbra::Collision {

    bool CollisionSystem::QueryCollisionsForTag(String _tag, Vector<Collision::CollisionResponse>& _outResponse) {
        return mCollisionEventResolverSystem->QueryCollisionsForTag(_tag, _outResponse);
    }

    bool CollisionSystem::QueryCollisionsForEntityID(EntityID _id, Vector<CollisionResponse>& _outResponse) {
        return mCollisionEventResolverSystem->QueryCollisionsForEntityID(_id, _outResponse);
    }

    void CollisionSystem::SetCollisionEventResolverSystem(CollisionEventResolverSystem* _collisionEventResolverSystem) {
        mCollisionEventResolverSystem = _collisionEventResolverSystem;
    }

    bool QueryCollisionsForTag(String _tag, Vector<CollisionResponse>& _outResponse) {
        return CollisionSystem::GetInstance()->QueryCollisionsForTag(_tag, _outResponse);
    }


    bool QueryCollisionsForEntityID(EntityID _id, Vector<CollisionResponse>& _outResponse) {
        return CollisionSystem::GetInstance()->QueryCollisionsForEntityID(_id, _outResponse);
    }

    CollisionSystem::CollisionSystem() {
        CollisionMatrix.emplace(ECollisionChannel::IGNORE, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::STATIC, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option2, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option3, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option4, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option5, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option6, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option7, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option8, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option9, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option10, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option11, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option12, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option13, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option14, CollisionMask());
        CollisionMatrix.emplace(ECollisionChannel::Option15, CollisionMask());
        for (uint8 i = 0; i < MAX_COLLISION_CHANNEL; i++) {
            for (uint8 j = i; j < MAX_COLLISION_CHANNEL; j++) {
                SetCollisionMask(static_cast<ECollisionChannel>(i), static_cast<ECollisionChannel>(j),
                    i != static_cast<uint8>(ECollisionChannel::IGNORE));
            }
        }
    }
} // namespace Umbra::Collision
