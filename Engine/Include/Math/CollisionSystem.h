#pragma once
#include "Math/CollisionConfig.h"

namespace Umbra::Collision {

    class CollisionSystem : public Singleton<CollisionSystem> {

    public:
        void SetCollisionEventResolverSystem(CollisionEventResolverSystem* _CollisionEventResolverSystem);

        inline static bool CheckCollisionMask(ECollisionChannel _channel1, ECollisionChannel _channel2) {
            return GetInstance()->CollisionMatrix[_channel1].test(static_cast<uint8>(_channel2));
        }

        inline void SetCollisionMask(ECollisionChannel _channel1, ECollisionChannel _channel2, bool _bCollides) {
            CollisionMatrix[_channel1].set(static_cast<uint8>(_channel2), _bCollides);
            CollisionMatrix[_channel2].set(static_cast<uint8>(_channel1), _bCollides);
        }

        bool QueryCollisionsForTag(String _tag, Vector<Collision::CollisionResponse>& _outResponse);
        bool QueryCollisionsForEntityID(EntityID _id, Vector<CollisionResponse>& _outResponse);

        CollisionSystem();

    private:
        UMap<ECollisionChannel, CollisionMask> CollisionMatrix;
        CollisionEventResolverSystem* mCollisionEventResolverSystem;
        friend class Singleton<CollisionSystem>;
    };

    bool QueryCollisionsForTag(String _tag, Vector<CollisionResponse>& _outResponse);
    bool QueryCollisionsForEntityID(EntityID _id, Vector<CollisionResponse>& _outResponse);

} // namespace Umbra::Collision
