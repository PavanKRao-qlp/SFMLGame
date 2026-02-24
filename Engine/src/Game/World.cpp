#include "Game/World.h"
#include "Game/PrefabManager.h"

#include "Core/AppWindow.h"
#include "ECS/Components/AudioSource.h"
#include "ECS/Components/CollisionCallback.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/SpriteQuad.h"
#include "Service/Audio/AudioServiceConfig.h"
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
        mWorldRegister->RegisterComponent<AudioSourceComponent>();

        IRenderDevice* renderDevice = GEngineStatics.AppWindowPtr->GetRenderDevice();

        mCameraSystem = std::make_shared<CameraSystem>(renderDevice);
        mCameraSystem->SetRenderSize(
            Math::Vector2f(GEngineStatics.GameConfig->WindowSize.x, GEngineStatics.GameConfig->WindowSize.y));
        Math::Vector2i windowSize = renderDevice->GetWindowSize();
        mCameraSystem->SetScreenSize(
            Math::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));

        mRenderSyncSystem = std::make_shared<RenderSyncSystem>();
        mPhysicsSystem    = std::make_shared<PhysicsSystem>();

        // Initialize Physics Service Layer
        PhysicsServiceConfig physicsConfig;
        mPhysicsService       = std::make_unique<PhysicsService>(physicsConfig);
        mPhysicsSyncSystem    = std::make_shared<PhysicsSyncSystem>(mPhysicsService.get());

        mWorldRegister->AddSystem(ESystemPhase::PreRender, 0, mCameraSystem);
        mWorldRegister->AddSystem(ESystemPhase::PreRender, 10, mRenderSyncSystem);
        mWorldRegister->AddSystem(ESystemPhase::Simulation, 0, mPhysicsSystem);
        // PhysicsSyncSystem runs after the old PhysicsSystem (lower priority = runs later)
        mWorldRegister->AddSystem(ESystemPhase::Simulation, 10, mPhysicsSyncSystem);

        // CollisionEventDispatchSystem runs after PhysicsSyncSystem
        mCollisionEventDispatchSystem = std::make_shared<CollisionEventDispatchSystem>(mPhysicsService.get());
        mWorldRegister->AddSystem(ESystemPhase::Simulation, 20, mCollisionEventDispatchSystem);

        // Initialize Audio Service Layer
        AudioServiceConfig audioConfig;
        mAudioService    = std::make_unique<AudioService>(audioConfig);
        mAudioSyncSystem = std::make_shared<AudioSyncSystem>(mAudioService.get());
        mWorldRegister->AddSystem(ESystemPhase::Simulation, 30, mAudioSyncSystem);
    }

    World::World() {
        mWorldRegister = std::make_shared<ECSRegister>();
        UMBRA_LOG_INFO("World Generated!");
    }

    World::~World() {
        mAudioSyncSystem.reset();
        mAudioService.reset();
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

    // ============== Audio Service Layer ==============

    AudioService* World::GetAudioService() {
        return mAudioService.get();
    }

    void World::PlaySound(const String& _filePath, ESoundGroup _group) {
        if (mAudioService) {
            mAudioService->PlaySound(_filePath, _group);
        }
    }

    void World::SetGroupVolume(ESoundGroup _group, float _volume) {
        if (mAudioService) {
            mAudioService->SetGroupVolume(_group, _volume);
        }
    }

    // ============== Prefab Layer ==============

    EntityID World::Instantiate(const String& _prefabName) {
        return PrefabManager::GetInstance()->Instantiate(_prefabName, *this);
    }

    EntityID World::Instantiate(const String& _prefabName, std::function<void(EntityID)> _overrideFn) {
        return PrefabManager::GetInstance()->Instantiate(_prefabName, *this, std::move(_overrideFn));
    }

} // namespace Umbra
