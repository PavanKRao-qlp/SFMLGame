#include "SfxScene.h"

#include "Asset/AssetManager.h"
#include "Audio/AudioSource.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "Umbra.h"
#include "imgui.h"


void SfxScene::Initialize() {}

void SfxScene::OnFixedUpdated() {}

void SfxScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }
    ImGui::Begin("Sfx showcase");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    if (ImGui::Button("Play")) {
        Umbra::SharedPtr<Umbra::AudioSource> sfx =
            Umbra::AssetManager::GetInstance()->Load<Umbra::AudioSource>("Asset/Audio/SFX/Report.wav");
        if (sfx != nullptr) {
            Umbra::AudioManager::GetInstance()::PlayOneShot(sfx);
        }
    }
    if (ImGui::Button("Stop")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    ImGui::SliderFloat("Volume", &mSfxVolume, 0, 10);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> SfxScene::InsatiateCopy() {
    return std::make_shared<SfxScene>(*this);
}

void SfxScene::OnBeginPlay() {}

void SfxScene::OnEndPlay() {}
