#include "Game/World.h"

#include "Core/AppWindow.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Systems/RenderSystem.h"

namespace Umbra {

    void World::InitializeCoreSystems() {
        mWorldRegister->RegisterComponent<SpriteComponent>();
        mWorldRegister->RegisterComponent<TransformComponent>();
        mWorldRegister->RegisterComponent<CameraComponent>();
        mWorldRegister->RegisterComponent<PhysicsBodyComponent>();
        mWorldRegister->RegisterComponent<BoxColliderComponent>();
        mWorldRegister->RegisterComponent<CircleColliderComponent>();

        IRenderDevice* renderDevice = GEngineStatics.AppWindowPtr->GetRenderDevice();

        mCameraSystem = std::make_shared<CameraSystem>(renderDevice);
        mCameraSystem->SetRenderSize(
            Math::Vector2f(GEngineStatics.GameConfig->WindowSize.x, GEngineStatics.GameConfig->WindowSize.y));
        Math::Vector2i windowSize = renderDevice->GetWindowSize();
        mCameraSystem->SetScreenSize(Math::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));

        mRenderSystem  = std::make_shared<RenderSystem>(renderDevice, GEngineStatics.ImGuiBackend);
        mPhysicsSystem = std::make_shared<PhysicsSystem>();

        mWorldRegister->AddSystem(ESystemPhase::PreRender, 0, mCameraSystem);
        mWorldRegister->AddSystem(ESystemPhase::Render, 0, mRenderSystem);
        mWorldRegister->AddSystem(ESystemPhase::Simulation, 0, mPhysicsSystem);
    }

    World::World() {
        mWorldRegister = std::make_shared<ECSRegister>();
        UMBRA_LOG_INFO("World Generated!");
    }

    World::~World() {
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

} // namespace Umbra
