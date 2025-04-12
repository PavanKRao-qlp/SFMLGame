#include "LandingScene.h"

#include "Core/Random.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "Umbra.h"
#include "imgui.h"

void LandingScene::Initialize() {
    if (GetCameraEntity() != Umbra::MAX_ENTITY) {
        GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(100);
    }
}

void LandingScene::OnFixedUpdated() {}

void LandingScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }
    ImGui::Begin("Physics Test Bed!");

    if (ImGui::Button("1: Point Line Segment Distance")) {
        GetSceneManager().GoToScene("Scene1");
    }
    if (ImGui::Button("2 Circle Line Segment Distance")) {
        GetSceneManager().GoToScene("Scene2");
    }
    if (ImGui::Button("3 Point/Circle AABB Segment Distance")) {
        GetSceneManager().GoToScene("Scene3");
    }
    if (ImGui::Button("4 Point/Circle OBB Segment Distance")) {
        GetSceneManager().GoToScene("Scene4");
    }
    if (ImGui::Button("5 Point/Circle Triangle Segment Distance")) {
        GetSceneManager().GoToScene("Scene5");
    }

    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> LandingScene::InsatiateCopy() {
    return std::make_shared<LandingScene>(*this);
}

void LandingScene::OnBeginPlay() {}

void LandingScene::OnEndPlay() {}
