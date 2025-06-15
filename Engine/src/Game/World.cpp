#include "Game/World.h"

#include "Core/AppWindow.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Systems/RenderSystem.h"

#include < SFML/Graphics/View.hpp>

namespace Umbra {

    // void World::SetECSRegister(ECSRegister* _worldRegister) {
    //     mWorldRegister = _worldRegister;
    // }

    // ECSRegister* World::GetRegister() {
    //     return mWorldRegister;
    // }

    // void World::FlushWorld() {
    //     mWorldRegister->FlushRegister();
    // }

    void World::InitializeCoreSystems() {
        mWorldRegister->RegisterComponent<SpriteComponent>();
        mWorldRegister->RegisterComponent<TransformComponent>();
        mWorldRegister->RegisterComponent<CameraComponent>();
        mWorldRegister->RegisterComponent<PhysicsBodyComponent>();
        mWorldRegister->RegisterComponent<BoxColliderComponent>();
        mWorldRegister->RegisterComponent<CircleColliderComponent>();
        mCameraSystem = std::make_shared<CameraSystem>(GEngineStatics.AppWindowPtr->GetRenderWindowView());
        mCameraSystem->SetRenderSize(
            Math::Vector2f(GEngineStatics.GameConfig->WindowSize.x, GEngineStatics.GameConfig->WindowSize.y));
        mCameraSystem->SetScreenSize(Math::Vector2f(GEngineStatics.AppWindowPtr->GetRenderWindowHandle()->getSize().x,
            GEngineStatics.AppWindowPtr->GetRenderWindowHandle()->getSize().y));
        mRenderSystem = std::make_shared<RenderSystem>(
            GEngineStatics.AppWindowPtr->GetRenderWindowHandle(), GEngineStatics.ImGuiBackend);
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
        sf::Vector2f worldPos = GEngineStatics.AppWindowPtr->GetRenderWindowHandle()->mapPixelToCoords(
            sf::Vector2i(_screenPos.x, _screenPos.y));
        return Math::Vector2f(worldPos.x, -worldPos.y);
    }

} // namespace Umbra
