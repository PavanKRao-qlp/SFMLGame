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
    ImGui::Begin("Audio Test Bed!");

    if (ImGui::Button("1: Play Sfx")) {
        GetSceneManager().GoToScene("Scene1");
    }
    if (ImGui::Button("2 Play Music")) {
        GetSceneManager().GoToScene("Scene2");
    }
    // if (ImGui::Button("11 BVH Demo")) {
    //     GetSceneManager().GoToScene("Scene5");
    // }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> LandingScene::InsatiateCopy() {
    return std::make_shared<LandingScene>(*this);
}

void LandingScene::OnBeginPlay() {}

void LandingScene::OnEndPlay() {}
