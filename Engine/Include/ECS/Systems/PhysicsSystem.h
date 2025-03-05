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
        Math::Vector2f GravityVector = Math::Vector2f(0, -9.8f);
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
                // sum all force applied to this object by f = ma;
                if (physicsBodyComponent->bAffectedByGravity) {
                    if (physicsBodyComponent->mInverseMass > 0) {
                        Math::Vector2f gravityForce =
                            mPhysicsWorldConfig.GravityVector / physicsBodyComponent->mInverseMass;
                        physicsBodyComponent->mForceAccumulated += gravityForce;
                    }
                }
                // calculate acceleration based on f = ma
                physicsBodyComponent->mAcceleration =
                    physicsBodyComponent->mForceAccumulated * physicsBodyComponent->mInverseMass;
                // Calculate displacement using s = vt + ((1/2) * at^2)
                transform->Position += (physicsBodyComponent->mVelocity * fixedDeltaTime)
                                     + (physicsBodyComponent->mAcceleration * Math::Pow(fixedDeltaTime, 2) * 0.5f);
                // applyAcceleration for next frame
                physicsBodyComponent->mVelocity += physicsBodyComponent->mAcceleration * fixedDeltaTime;
                // Apply Damping
                physicsBodyComponent->mVelocity *= Math::Pow(mDamping, fixedDeltaTime);
                // reset all force accumulation
                physicsBodyComponent->mForceAccumulated = Math::Vector2f(0, 0);
            }
        }

    protected:
        PhysicsWorldConfig mPhysicsWorldConfig;
        // * Fake linear Damping added to motion to counter the numerical instability during integrating steps
        double mDamping = 0.9999;
    };
} // namespace Umbra
