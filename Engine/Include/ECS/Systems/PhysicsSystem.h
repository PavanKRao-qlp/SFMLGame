#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/Rigidbody.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"

namespace Umbra {
    class PhysicsSystem : public System {
    public:
        inline PhysicsSystem() : System(new ECView<RigidBodyComponent, TransformComponent>()) {};
        inline ~PhysicsSystem() {};
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                TransformComponent* transform          = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                RigidBodyComponent* rigidBodyComponent = mView->ecsRegister->GetComponent<RigidBodyComponent>(entity);
                float velocityMag = hlslpp::dot(rigidBodyComponent->Velocity, rigidBodyComponent->Velocity);
                if (velocityMag > 0) {
                    transform->Position += rigidBodyComponent->Velocity;
                }
            }
        }
    };
} // namespace Umbra
