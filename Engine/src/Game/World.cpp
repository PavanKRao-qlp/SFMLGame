#include "Game/World.h"

#include "Core/AppWindow.h"
#include "ECS/Components/CollisionCallback.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Systems/RenderSystem.h"
#include "Service/Physics/PhysicsServiceConfig.h"

namespace Umbra {

    void World::InitializeCoreSystems() {
        mWorldRegister->RegisterComponent<SpriteComponent>();
        mWorldRegister->RegisterComponent<TransformComponent>();
        mWorldRegister->RegisterComponent<CameraComponent>();
        mWorldRegister->RegisterComponent<PhysicsBodyComponent>();
        mWorldRegister->RegisterComponent<BoxColliderComponent>();
        mWorldRegister->RegisterComponent<CircleColliderComponent>();
        mWorldRegister->RegisterComponent<RigidbodyHandleComponent>();
        mWorldRegister->RegisterComponent<LifeTimeComponent>();
        mWorldRegister->RegisterComponent<CollisionCallbackComponent>();

        IRenderDevice* renderDevice = GEngineStatics.AppWindowPtr->GetRenderDevice();

        mCameraSystem = std::make_shared<CameraSystem>(renderDevice);
        mCameraSystem->SetRenderSize(
            Math::Vector2f(GEngineStatics.GameConfig->WindowSize.x, GEngineStatics.GameConfig->WindowSize.y));
        Math::Vector2i windowSize = renderDevice->GetWindowSize();
        mCameraSystem->SetScreenSize(
            Math::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));

        mRenderSystem  = std::make_shared<RenderSystem>(renderDevice, GEngineStatics.ImGuiBackend);
        mPhysicsSystem = std::make_shared<PhysicsSystem>();

        // Initialize Physics Service Layer
        PhysicsServiceConfig physicsConfig;
        mPhysicsService       = std::make_unique<PhysicsService>(physicsConfig);
        mPhysicsSyncSystem    = std::make_shared<PhysicsSyncSystem>(mPhysicsService.get());

        mWorldRegister->AddSystem(ESystemPhase::PreRender, 0, mCameraSystem);
        mWorldRegister->AddSystem(ESystemPhase::Render, 0, mRenderSystem);
        mWorldRegister->AddSystem(ESystemPhase::Simulation, 0, mPhysicsSystem);
        // PhysicsSyncSystem runs after the old PhysicsSystem (lower priority = runs later)
        mWorldRegister->AddSystem(ESystemPhase::Simulation, 10, mPhysicsSyncSystem);

        // CollisionEventDispatchSystem runs after PhysicsSyncSystem
        mCollisionEventDispatchSystem = std::make_shared<CollisionEventDispatchSystem>(mPhysicsService.get());
        mWorldRegister->AddSystem(ESystemPhase::Simulation, 20, mCollisionEventDispatchSystem);
    }

    World::World() {
        mWorldRegister = std::make_shared<ECSRegister>();
        UMBRA_LOG_INFO("World Generated!");
    }

    World::~World() {
        mPhysicsSyncSystem.reset();
        mWorldRegister.reset();
        mPhysicsService.reset();
        UMBRA_LOG_INFO("World Destroyed!");
    }

    void World::Update() {
        mWorldRegister->CleanUp();
    }

    void World::Simulate() {
        mWorldRegister->Update(ESystemPhase::Simulation);
    }

    void World::Render() {
        mWorldRegister->Update(ESystemPhase::FrameStart);
        mWorldRegister->Update(ESystemPhase::PreRender);
        mWorldRegister->Update(ESystemPhase::Render);
        mWorldRegister->Update(ESystemPhase::FrameEnd);
    }

    PhysicsSystem* World::GetPhysicsSystem() {
        return mPhysicsSystem.get();
    }

    Math::Vector2f World::GetScreenToWorldPosition(Math::Vector2i _screenPos) {
        IRenderDevice* renderDevice = GEngineStatics.AppWindowPtr->GetRenderDevice();
        Math::Vector2f worldPos     = renderDevice->MapPixelToCoords(_screenPos);
        return Math::Vector2f(worldPos.x, -worldPos.y);
    }

    // ============== Physics Service Layer ==============

    PhysicsService* World::GetPhysicsService() {
        return mPhysicsService.get();
    }

    void World::ApplyForce(EntityID _entity, Math::Vector2f _force) {
        if (!mPhysicsService || !mWorldRegister->HasComponent<RigidbodyHandleComponent>(_entity)) {
            return;
        }
        RigidbodyHandleComponent* rb = mWorldRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        if (rb && rb->HasValidBody()) {
            mPhysicsService->ApplyForce(rb->Handle, _force);
        }
    }

    void World::ApplyForceAtPoint(EntityID _entity, Math::Vector2f _force, Math::Vector2f _worldPoint) {
        if (!mPhysicsService || !mWorldRegister->HasComponent<RigidbodyHandleComponent>(_entity)) {
            return;
        }
        RigidbodyHandleComponent* rb = mWorldRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        if (rb && rb->HasValidBody()) {
            mPhysicsService->ApplyForceAtPoint(rb->Handle, _force * 50, _worldPoint);
        }
    }

    void World::ApplyImpulse(EntityID _entity, Math::Vector2f _impulse) {
        if (!mPhysicsService || !mWorldRegister->HasComponent<RigidbodyHandleComponent>(_entity)) {
            return;
        }
        RigidbodyHandleComponent* rb = mWorldRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        if (rb && rb->HasValidBody()) {
            mPhysicsService->ApplyImpulse(rb->Handle, _impulse);
        }
    }

    void World::ApplyTorque(EntityID _entity, float _torque) {
        if (!mPhysicsService || !mWorldRegister->HasComponent<RigidbodyHandleComponent>(_entity)) {
            return;
        }
        RigidbodyHandleComponent* rb = mWorldRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        if (rb && rb->HasValidBody()) {
            mPhysicsService->ApplyTorque(rb->Handle, _torque);
        }
    }

    void Umbra::World::ApplyAngularImpulse(EntityID _entity, float _impulse) {
        if (!mPhysicsService || !mWorldRegister->HasComponent<RigidbodyHandleComponent>(_entity)) {
            return;
        }
        RigidbodyHandleComponent* rb = mWorldRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        if (rb && rb->HasValidBody()) {
            mPhysicsService->ApplyAngularImpulse(rb->Handle, _impulse);
        }
    }


    void World::SetVelocity(EntityID _entity, Math::Vector2f _velocity) {
        if (!mPhysicsService || !mWorldRegister->HasComponent<RigidbodyHandleComponent>(_entity)) {
            return;
        }
        RigidbodyHandleComponent* rb = mWorldRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        if (rb && rb->HasValidBody()) {
            mPhysicsService->SetVelocity(rb->Handle, _velocity);
        }
    }

    Math::Vector2f World::GetVelocity(EntityID _entity) {
        if (!mWorldRegister->HasComponent<RigidbodyHandleComponent>(_entity)) {
            return Math::Vector2f(0, 0);
        }
        RigidbodyHandleComponent* rb = mWorldRegister->GetComponent<RigidbodyHandleComponent>(_entity);
        if (rb) {
            // Use cached value for efficiency
            return rb->CachedVelocity;
        }
        return Math::Vector2f(0, 0);
    }

} // namespace Umbra
