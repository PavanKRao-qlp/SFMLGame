#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/PhysicsBodyComponent.h"
#include "ECS/Components/Rigidbody.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Math/MathUtils.h"

namespace Umbra {
#if PHYSICS_OLD
    class PhysicsSystem : public System {
    public:
        inline PhysicsSystem() : System(new ECView<RigidBodyComponent, TransformComponent>()) {}
        inline ~PhysicsSystem() {}
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                TransformComponent* transform          = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                RigidBodyComponent* rigidBodyComponent = mView->ecsRegister->GetComponent<RigidBodyComponent>(entity);
                float velocityMag                      = rigidBodyComponent->Velocity.Magnitude();
                if (velocityMag > 0) {
                    transform->Position += (rigidBodyComponent->Velocity * EngineTime::GetDeltaTime());
                }
            }
        }
    };
#endif
    struct PhysicsWorldConfig {
    public:
        Math::Vector2f GravityVector;
    };

    class PhysicsSystem : public System {
    public:
        inline PhysicsSystem() : System(new ECView<PhysicsBodyComponent, TransformComponent>()) {}
        inline ~PhysicsSystem() {}
        inline void Update() override {
            float fixedDeltaTime = GEngineStatics.GameConfig->FixedDeltaTime;
            for (EntityID entity : mView->mEntities) {
                TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                PhysicsBodyComponent* physicsBodyComponent =
                    mView->ecsRegister->GetComponent<PhysicsBodyComponent>(entity);
                // Calculate displacement using s = vt + ((1/2) * at^2)
                // transform->Position += (physicsBodyComponent->mVelocity * fixedDeltaTime)
                //                      + (PhysicsBodyComponent->mAcceleration * Math::Pow(fixedDeltaTime, 2) * 0.5f);
                // physicsBodyComponent->mVelocity += physicsBodyComponent->mAcceleration * fixedDeltaTime;
                // physicsBodyComponent->mVelocity *= Math::Pow(mDamping, fixedDeltaTime);
            }
        }

    protected:
        PhysicsWorldConfig mPhysicsWorldConfig;
        // * Fake linear Damping added to motion to counter the numerical instability during integrating steps
        double mDamping = 0.9999;
    };
} // namespace Umbra
