#include "MainScene.h"

#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Umbra.h"

void MainScene::Initialize() {
    CreateDefaultCamera(100.0f);
}

void MainScene::OnFixedUpdate() {}

void MainScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }
    Umbra::RenderSystem::DebugDrawCircle(Umbra::Math::Vector2f(0.0f, 0.0f), mCircleRadius, true, Umbra::Color::White);
}

Umbra::SharedPtr<Umbra::Scene> MainScene::InstantiateCopy() {
    return std::make_shared<MainScene>(*this);
}

void MainScene::OnBeginPlay() {}

void MainScene::OnEndPlay() {}
