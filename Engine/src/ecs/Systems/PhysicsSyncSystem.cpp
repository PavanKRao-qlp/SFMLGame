#include "ECS/Systems/PhysicsSyncSystem.h"

#include "ECS/Components/Collider.h"
#include "Umbra.h"

namespace Umbra {

    PhysicsSyncSystem::PhysicsSyncSystem(PhysicsService* _physicsService)
        : System(std::make_unique<ECView<RigidbodyHandleComponent, TransformComponent>>()),
          mPhysicsService(_physicsService) {}

    PhysicsSyncSystem::~PhysicsSyncSystem() {
        // Destroy all physics bodies owned by entities in our view
        for (EntityID entity : mView->mEntities) {
            DestroyBodyForEntity(entity);
        }
    }

    void PhysicsSyncSystem::Update() {
        if (mPhysicsService == nullptr) {
            return;
        }

        float fixedDeltaTime = GEngineStatics.GameConfig->FixedDeltaTime;

        // Phase 1: Create bodies for entities that need them
        for (EntityID entity : mView->mEntities) {
            RigidbodyHandleComponent* rb = mView->ecsRegister->GetComponent<RigidbodyHandleComponent>(entity);
            if (rb->bNeedsBodyCreation) {
                CreateBodyForEntity(entity);
            }
        }

        // Phase 2: Push kinematic transforms TO physics service
        SyncKinematicToPhysics();
        SyncRigidbodyPropertyChanges();
        // Phase 3: Step the physics simulation
        mPhysicsService->Step(fixedDeltaTime);

        // Phase 4: Pull dynamic transforms FROM physics service
        SyncPhysicsToDynamic();
    }

    void PhysicsSyncSystem::AddEntity(EntityID _entity) {
        System::AddEntity(_entity);

        // Mark entity as needing body creation
        RigidbodyHandleComponent* rb = mView->ecsRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        if (rb) {
            rb->bNeedsBodyCreation = true;
        }
    }

    void PhysicsSyncSystem::RemoveEntity(EntityID _entity) {
        DestroyBodyForEntity(_entity);
        System::RemoveEntity(_entity);
    }

    void PhysicsSyncSystem::CreateBodyForEntity(EntityID _entity) {
        if (mPhysicsService == nullptr) {
            return;
        }

        RigidbodyHandleComponent* rb  = mView->ecsRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(_entity);


        if (rb == nullptr || transform == nullptr) {
            return;
        }

        // If already has a valid body, skip
        if (rb->Handle.IsValid() && mPhysicsService->IsBodyValid(rb->Handle)) {
            rb->bNeedsBodyCreation = false;
            return;
        }

        // Create body definition from component data
        BodyDef def;
        def.Position           = transform->Position;
        def.Angle              = transform->Angle;
        def.Velocity           = rb->CachedVelocity;
        def.AngularVelocity    = rb->CachedAngularVelocity;
        def.Mass               = rb->Mass;
        def.Inertia            = rb->Inertia;
        def.LinearDamping      = rb->LinearDamping;
        def.AngularDamping     = rb->AngularDamping;
        def.CoefOfRestitution  = rb->CoefOfRestitution;
        def.StaticFriction     = rb->StaticFriction;
        def.DynamicFriction    = rb->DynamicFriction;
        def.bAffectedByGravity = rb->bAffectedByGravity;
        def.bIsKinematic       = rb->bIsKinematic;
        def.ShapeData          = ShapeData();

        if (mView->ecsRegister->HasComponent<BoxColliderComponent>(_entity)) {

            BoxColliderComponent* BoxCollider = mView->ecsRegister->GetComponent<BoxColliderComponent>(_entity);
            def.ShapeData                     = ShapeData::MakeBox(BoxCollider->Size);
        }
        if (mView->ecsRegister->HasComponent<CircleColliderComponent>(_entity)) {
            CircleColliderComponent* CircleCollider =
                mView->ecsRegister->GetComponent<CircleColliderComponent>(_entity);
            def.ShapeData = ShapeData::MakeCircle(CircleCollider->Radius);
        }
        // Store EntityID in UserData for reverse lookup
        def.UserData = reinterpret_cast<void*>(static_cast<uintptr_t>(_entity));

        // Create body in physics service
        rb->Handle             = mPhysicsService->CreateBody(def);
        rb->bNeedsBodyCreation = false;
    }

    void PhysicsSyncSystem::DestroyBodyForEntity(EntityID _entity) {
        if (mPhysicsService == nullptr) {
            return;
        }

        RigidbodyHandleComponent* rb = mView->ecsRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        if (rb == nullptr) {
            return;
        }

        if (rb->Handle.IsValid()) {
            mPhysicsService->DestroyBody(rb->Handle);
            rb->Handle = BodyHandle::Invalid();
        }
    }

    void PhysicsSyncSystem::SyncKinematicToPhysics() {
        for (EntityID entity : mView->mEntities) {
            RigidbodyHandleComponent* rb = mView->ecsRegister->GetComponent<RigidbodyHandleComponent>(entity);

            if (!rb->HasValidBody() || !rb->bSyncEnabled) {
                continue;
            }

            // Only sync kinematic bodies TO physics
            if (rb->bIsKinematic) {
                TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                mPhysicsService->SetPosition(rb->Handle, transform->Position);
                mPhysicsService->SetAngle(rb->Handle, transform->Angle);
            }
        }
    }

    void PhysicsSyncSystem::SyncRigidbodyPropertyChanges() {
        for (EntityID entity : mView->mEntities) {
            RigidbodyHandleComponent* rb = mView->ecsRegister->GetComponent<RigidbodyHandleComponent>(entity);

            if (!rb->HasValidBody() || !rb->bSyncEnabled) {
                continue;
            }
            mPhysicsService->SetMass(rb->Handle, rb->Mass);
            mPhysicsService->SetCoefOfRestitution(rb->Handle, rb->CoefOfRestitution);
            mPhysicsService->SetStaticFriction(rb->Handle, rb->StaticFriction);
            mPhysicsService->SetDynamicFriction(rb->Handle, rb->DynamicFriction);
        }
    }


    void PhysicsSyncSystem::SyncPhysicsToDynamic() {
        for (EntityID entity : mView->mEntities) {
            RigidbodyHandleComponent* rb = mView->ecsRegister->GetComponent<RigidbodyHandleComponent>(entity);

            if (!rb->HasValidBody() || !rb->bSyncEnabled) {
                continue;
            }

            // Sync dynamic bodies FROM physics
            if (!rb->bIsKinematic) {
                TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);

                // Pull position and angle from physics
                transform->Position = mPhysicsService->GetPosition(rb->Handle);
                transform->Angle    = mPhysicsService->GetAngle(rb->Handle);

                // Cache velocity for efficient reads by game code
                rb->CachedVelocity        = mPhysicsService->GetVelocity(rb->Handle);
                rb->CachedAngularVelocity = mPhysicsService->GetAngularVelocity(rb->Handle);
            }
        }
    }

} // namespace Umbra
