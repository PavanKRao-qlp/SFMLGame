#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/PhysicsBodyComponent.h"
#include "ECS/Components/Rigidbody.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "ECS/Systems/RenderSystem.h"
#include "Math/MathUtils.h"
#include "Physics/Collision.h"
#include "Physics/ForceGenerator.h"

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
        inline PhysicsSystem() : System(new ECView<PhysicsBodyComponent, TransformComponent>()) {
            mContactResolver              = std::make_unique<ContactResolver>();
            mCollisionDetector            = std::make_unique<CollisionDetector>();
            mCollisionDetector->mBaseView = (mView);
            mContactResolver->mBaseView   = (mView);
        }
        inline ~PhysicsSystem() {}
        inline void Update() override {
            float fixedDeltaTime = GEngineStatics.GameConfig->FixedDeltaTime;
            //  apply the force generators
            for (SharedPtr<IForceGenerator> forceGenerator : mForceGenerators) {
                forceGenerator->ApplyForce(fixedDeltaTime);
            }
            // Then integrate the objects
            for (EntityID entity : mView->mEntities) {
                TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                PhysicsBodyComponent* physicsBodyComponent =
                    mView->ecsRegister->GetComponent<PhysicsBodyComponent>(entity);

                // RenderSystem::DebugDrawLine(transform->Position,
                //  transform->Position + physicsBodyComponent->mForceAccumulated, sf::Color::Blue);

                // sum all force applied to this object by f = ma;
                if (physicsBodyComponent->bAffectedByGravity) {
                    if (physicsBodyComponent->mInverseMass > 0) {
                        Math::Vector2f gravityForce =
                            mPhysicsWorldConfig.GravityVector / physicsBodyComponent->mInverseMass;
                        physicsBodyComponent->ApplyForce(gravityForce);
                    }
                }

                // calculate acceleration based on f = ma
                Math::Vector2f acceleration =
                    physicsBodyComponent->mForceAccumulated * physicsBodyComponent->mInverseMass;
                double angularAcceleration =
                    physicsBodyComponent->mTorqueAccumulated * physicsBodyComponent->mInverseInertia;

                // Calculate displacement using s = vt + ((1/2) * at^2)
                Math::Vector2f position = transform->Position + (physicsBodyComponent->mVelocity * fixedDeltaTime)
                                        + (physicsBodyComponent->mAcceleration * Math::Pow(fixedDeltaTime, 2) * 0.5f);
                // Calculate angular displacement using θ = ωt + 0.5αt^2
                double angle = Math::DegreeToRadian(transform->Angle)
                             + (physicsBodyComponent->mAngularVelocity * fixedDeltaTime)
                             + (physicsBodyComponent->mAngularAcceleration * Math::Pow(fixedDeltaTime, 2) * 0.5f);
                // cap angle to 0-360
                angle = Math::Fmod(angle, (Math::PI * 2.f));
                // apply linear Acceleration for next frame
                Math::Vector2f velocity =
                    physicsBodyComponent->mVelocity
                    + ((physicsBodyComponent->mAcceleration + acceleration) * 0.5f * fixedDeltaTime);
                // apply angular Acceleration for next frame
                float angularVelocity =
                    physicsBodyComponent->mAngularVelocity
                    + (physicsBodyComponent->mAngularAcceleration + angularAcceleration) * 0.5f * fixedDeltaTime;

                transform->Position                 = position;
                transform->Angle                    = Math::RadianToDegree(angle);
                physicsBodyComponent->mVelocity     = velocity;
                physicsBodyComponent->mAcceleration = acceleration;
                // Apply Damping
                physicsBodyComponent->mVelocity *= Math::Pow(mLinearDamping, fixedDeltaTime);
                physicsBodyComponent->mAngularVelocity *= Math::Pow(mAngularDamping, fixedDeltaTime);


                //  reset all force accumulation
                physicsBodyComponent->mForceAccumulated  = Math::Vector2f(0, 0);
                physicsBodyComponent->mTorqueAccumulated = 0;
            }
            // solve collisions
            Vector<Tuple<EntityID, EntityID>> PossibleCollisions = mCollisionDetector->RunBroadPhase();
            Vector<Collision> collisions = mCollisionDetector->RunNarrowPhase(PossibleCollisions);
            mContactResolver->ResolveContacts(collisions, fixedDeltaTime);
        }

        inline void AddForceGenerator(SharedPtr<IForceGenerator> forceGenerator) {
            forceGenerator->ProvideECSView(mView);
            mForceGenerators.emplace_back(forceGenerator);
        }
        inline void RemoveForceGenerator(SharedPtr<IForceGenerator>* forceGenerator) {
            // mForceGenerators.erase(forceGenerator);
        }

    protected:
        Vector<SharedPtr<IForceGenerator>> mForceGenerators;
        Vector<SharedPtr<ITorqueGenerator>> mTorqueGenerators;
        PhysicsWorldConfig mPhysicsWorldConfig;

        // * Fake linear Damping added to motion to counter the numerical instability during integrating steps
        double mLinearDamping = 0.975; // 0.975;
        // * Fake Angular Damping added to motion to counter the numerical instability during integrating steps
        double mAngularDamping = 0.975; // 0.975;
        //
        UniquePtr<ContactResolver> mContactResolver;
        UniquePtr<CollisionDetector> mCollisionDetector;
    };

} // namespace Umbra
