#pragma once
// Old physics system — superseded by PhysicsService + PhysicsSyncSystem.
// All code below is retained for reference only.

// #include "Core/Clock.h"
// #include "ECS/Component.h"
// #include "ECS/Components/BoundingVolume.h"
// #include "ECS/Components/PhysicsBodyComponent.h"
// #include "ECS/Components/Rigidbody.h"
// #include "ECS/Components/Transform.h"
// #include "ECS/Enity.h"
// #include "ECS/System.h"
// #include "Service/ServiceLocator.h"
// #include "Math/MathUtils.h"
// #include "Physics/Collision.h"
// #include "Physics/CollisionDetector.h"
// #include "Physics/ContactResolver.h"
// #include "Physics/ForceGenerator.h"
//
// namespace Umbra {
// #if PHYSICS_OLD
//     class PhysicsSystem : public System {
//     public:
//         inline PhysicsSystem() : System(std::make_unique<ECView<RigidBodyComponent, TransformComponent>>()) {}
//         inline ~PhysicsSystem() {}
//         inline void Update() override {
//             for (EntityID entity : mView->mEntities) {
//                 TransformComponent* transform          = mView->ecsRegister->GetComponent<TransformComponent>(entity);
//                 RigidBodyComponent* rigidBodyComponent = mView->ecsRegister->GetComponent<RigidBodyComponent>(entity);
//                 float velocityMag                      = rigidBodyComponent->Velocity.Magnitude();
//                 if (velocityMag > 0) {
//                     transform->Position += (rigidBodyComponent->Velocity * EngineTime::GetDeltaTime());
//                 }
//             }
//         }
//     };
// #endif
//     struct PhysicsWorldConfig {
//     public:
//         Math::Vector2f GravityVector = Math::Vector2f(0, -9.8f);
//     };
//
//     class PhysicsSystem : public System {
//     public:
//         // (entire implementation commented out — see PhysicsService + PhysicsSyncSystem)
//     };
//
// } // namespace Umbra
