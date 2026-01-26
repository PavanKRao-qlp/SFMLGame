#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/BoundingVolume.h"
#include "ECS/Components/PhysicsBodyComponent.h"
#include "ECS/Components/Rigidbody.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "ECS/Systems/RenderSystem.h"
#include "Math/MathUtils.h"
#include "Physics/Collision.h"
#include "Physics/CollisionDetector.h"
#include "Physics/ContactResolver.h"
#include "Physics/ForceGenerator.h"

namespace Umbra {
#if PHYSICS_OLD
    class PhysicsSystem : public System {
    public:
        inline PhysicsSystem() : System(std::make_unique<ECView<RigidBodyComponent, TransformComponent>>()) {}
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
        inline PhysicsSystem() : System(std::make_unique<ECView<PhysicsBodyComponent, TransformComponent>>()) {
            mContactResolver              = std::make_unique<ContactResolver>();
            mCollisionDetector            = std::make_unique<CollisionDetector>();
            mCollisionDetector->mBaseView = mView.get();
            mContactResolver->mBaseView   = mView.get();
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
            // Add Bounding Volume to objects if they dont have
            BuildBoundingVolumeForEntity();
            //

            // solve collisions
            Vector<Tuple<EntityID, EntityID>> PossibleCollisions = mCollisionDetector->RunBroadPhase();
            Vector<Collision> collisions = mCollisionDetector->RunNarrowPhase(PossibleCollisions);
            mContactResolver->ResolveContacts(collisions, fixedDeltaTime);
        }

        inline void AddForceGenerator(SharedPtr<IForceGenerator> forceGenerator) {
            forceGenerator->ProvideECSView(mView.get());
            mForceGenerators.emplace_back(forceGenerator);
        }
        inline void RemoveForceGenerator(SharedPtr<IForceGenerator>* forceGenerator) {
            // mForceGenerators.erase(forceGenerator);
        }

        inline void BuildBoundingVolumeForEntity() {

            mCollisionDetector->ClearBoundingVolumeSpatialData();
            for (EntityID entity : mView->mEntities) {
                TransformComponent* transform      = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                BoundingVolumeAABB* boundingVolume = nullptr;
                bool bRebuilt                      = false;
                if (mView->ecsRegister->HasComponent<BoxColliderComponent>(entity)) {
                    boundingVolume                    = &BoundingVolumeAABB();
                    boundingVolume->Id                = entity;
                    BoxColliderComponent* boxCollider = mView->ecsRegister->GetComponent<BoxColliderComponent>(entity);
                    Math::Vector2f extent             = boxCollider->Size;
                    float angleRad                    = Math::DegreeToRadian(transform->Angle);
                    extent.x = Math::Abs(extent.x * Math::Cos(angleRad)) + Math::Abs(extent.y * Math::Sin(angleRad));
                    extent.y = Math::Abs(extent.x * Math::Sin(angleRad)) + Math::Abs(extent.y * Math::Cos(angleRad));
                    boundingVolume->SlimBounds = Math::Bounds2D(transform->Position + boxCollider->Offset, extent);
                    //  if (!boundingVolume->FatBounds.Contains(boundingVolume->SlimBounds)) {
                    boundingVolume->FatBounds =
                        Math::Bounds2D(transform->Position + boxCollider->Offset, extent * 1.25f);
                    // bRebuilt = true;
                    //}
                } else if (mView->ecsRegister->HasComponent<CircleColliderComponent>(entity)) {
                    boundingVolume     = &BoundingVolumeAABB();
                    boundingVolume->Id = entity;
                    CircleColliderComponent* circleColliderComponent =
                        mView->ecsRegister->GetComponent<CircleColliderComponent>(entity);
                    Math::Vector2f extent =
                        Math::Vector2f(circleColliderComponent->Radius * 2, circleColliderComponent->Radius * 2);
                    boundingVolume->SlimBounds =
                        Math::Bounds2D(transform->Position + circleColliderComponent->Offset, extent);
                    boundingVolume->FatBounds =
                        Math::Bounds2D(transform->Position + circleColliderComponent->Offset, extent * 1.25f);
                }
                if (boundingVolume != nullptr) {
                    mCollisionDetector->AddBoundingVolume(*boundingVolume);
                    RenderSystem::DrawDebugBox(boundingVolume->FatBounds, bRebuilt, sf::Color::Red);
                    RenderSystem::DrawDebugBox(boundingVolume->SlimBounds, false, sf::Color::Yellow);
                }
                /*
                make a Bounding Volume data
                update data with transform
                update BV with new bounds
                BV build
                -> broadphase resolve
                */
            }
        }

        inline bool QueryColliderAt(Math::Vector2f _point) {
            if (mCollisionDetector->BroadphaseQueryColliderAt(_point)) {
                return true;
            }
            return false;
        }

        inline bool QueryCollidersInsideAABB(Math::Bounds2D _bound) {
            if (mCollisionDetector->BroadphaseQueryCollidersInsideAABB(_bound)) {
                return true;
            }
            return false;
        }

        inline bool Raycast(Math::Ray2D _ray) {
            if (mCollisionDetector->BroadphaseRayCast(_ray)) {
                return true;
            }
            return false;
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
