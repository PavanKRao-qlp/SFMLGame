#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/CollisionBox.h"
#include "ECS/Components/CollisionEvent.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"

namespace Umbra {
    class CollisionSystem : public System {
    public:
        inline CollisionSystem() : System(new ECView<CollisionBoxComponent, TransformComponent>()) {};
        inline ~CollisionSystem() {};
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                for (EntityID entityOther : mView->mEntities) {
                    if (entity == entityOther) {
                        continue;
                    }
                    TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                    CollisionBoxComponent* collisionBox =
                        mView->ecsRegister->GetComponent<CollisionBoxComponent>(entity);
                    TransformComponent* transformOther =
                        mView->ecsRegister->GetComponent<TransformComponent>(entityOther);
                    CollisionBoxComponent* collisionBoxOther =
                        mView->ecsRegister->GetComponent<CollisionBoxComponent>(entityOther);
                    Math::Bounds2D transformedBounds(
                        transform->Position + collisionBox->Bounds2D.Center, collisionBox->Bounds2D.Size);
                    Math::Bounds2D transformedBoundsOther(transformOther->Position + collisionBoxOther->Bounds2D.Center,
                        collisionBoxOther->Bounds2D.Size);
                    if (transformedBounds.Intersects(transformedBoundsOther)) {
                        Logger::Log(LogType::Trace, "Collision b.w %llu %llu ", entity, entityOther);
                    }
                }
            }
        }
    };
} // namespace Umbra
