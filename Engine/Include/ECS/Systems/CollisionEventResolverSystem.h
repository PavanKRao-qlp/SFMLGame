#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/CollisionBox.h"
#include "ECS/Components/CollisionEvent.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Math/CollisionSystem.h"

namespace Umbra {
    class CollisionEventResolverSystem : public System {

    public:
        inline CollisionEventResolverSystem() : System(new ECView<CollisionEventComponent>()) {
            Collision::CollisionSystem::GetInstance()->SetCollisionEventResolverSystem(this);
        }
        inline ~CollisionEventResolverSystem() {};
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                CollisionEventComponent* collisionEvent =
                    mView->ecsRegister->GetComponent<CollisionEventComponent>(entity);
                mView->ecsRegister->DestroyEntity(entity);
            }
        }

        inline bool QueryCollisionsForTag(String _tag, Vector<Collision::CollisionResponse>& _outResponse) {
            Vector<Collision::CollisionResponse> result;
            for (EntityID entity : mView->mEntities) {
                CollisionEventComponent* collisionEvent =
                    mView->ecsRegister->GetComponent<CollisionEventComponent>(entity);
                if (mView->ecsRegister->IsTag(collisionEvent->mCollisionData.mEntityA, _tag)
                    || mView->ecsRegister->IsTag(collisionEvent->mCollisionData.mEntityB, _tag)) {
                    result.emplace_back(collisionEvent->mCollisionData);
                };
            }
            _outResponse = result;
            return result.size() > 0;
        }

        inline bool QueryCollisionsForEntityID(EntityID _id, Vector<Collision::CollisionResponse>& _outResponse) {
            Vector<Collision::CollisionResponse> result;
            for (EntityID entity : mView->mEntities) {
                CollisionEventComponent* collisionEvent =
                    mView->ecsRegister->GetComponent<CollisionEventComponent>(entity);
                if (collisionEvent->mCollisionData.mEntityA == _id || collisionEvent->mCollisionData.mEntityB == _id) {
                    result.emplace_back(collisionEvent->mCollisionData);
                }
            }
            _outResponse = result;
            return result.size() > 0;
        }
    };
} // namespace Umbra
