#include "AudioDemoScene.h"

#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "Service/Audio/AudioService.h"
#include "Umbra.h"
#include "imgui.h"

void AudioDemoScene::Initialize() {
    CreateDefaultCamera(100.0f);
}

void AudioDemoScene::OnBeginPlay() {}

void AudioDemoScene::OnEndPlay() {
    // Stop music when leaving scene
    Umbra::AudioService* audio = GetWorld()->GetAudioService();
    if (audio && mMusicHandle.IsValid()) {
        audio->StopSound(mMusicHandle);
        mMusicHandle = Umbra::SoundHandle::Invalid();
        bMusicPlaying = false;
    }
}

void AudioDemoScene::OnFixedUpdate() {}

void AudioDemoScene::OnUpdate() {
    using namespace Umbra;

    if (Input::GetKey(KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }

    AudioService* audio = GetWorld()->GetAudioService();
    if (audio == nullptr) {
        return;
    }

    ImGui::Begin("Audio TestBed");

    // ============== Music Controls ==============
    ImGui::SeparatorText("Music");
    ImGui::Text("File: Asset/Audio/some.ogg");

    if (!bMusicPlaying) {
        if (ImGui::Button("Play Music")) {
            mMusicHandle = audio->PlaySound("Asset/Audio/some.ogg", ESoundGroup::Music, true);
            bMusicPlaying = true;
        }
    } else {
        if (ImGui::Button("Stop Music")) {
            audio->StopSound(mMusicHandle);
            mMusicHandle = SoundHandle::Invalid();
            bMusicPlaying = false;
        }
    }

    // Check if music finished externally
    if (bMusicPlaying && !audio->IsSoundValid(mMusicHandle)) {
        bMusicPlaying = false;
        mMusicHandle = SoundHandle::Invalid();
    }

    // ============== SFX Controls ==============
    ImGui::SeparatorText("SFX");
    ImGui::Text("File: Asset/Audio/Swing_000.wav");

    if (ImGui::Button("Play SFX")) {
        audio->PlaySound("Asset/Audio/Swing_000.wav", ESoundGroup::SFX);
    }

    // ============== Volume Controls ==============
    ImGui::SeparatorText("Volume");

    if (ImGui::SliderFloat("Master", &mMasterVolume, 0.0f, 1.0f)) {
        audio->SetGroupVolume(ESoundGroup::Master, mMasterVolume);
    }
    if (ImGui::SliderFloat("Music", &mMusicVolume, 0.0f, 1.0f)) {
        audio->SetGroupVolume(ESoundGroup::Music, mMusicVolume);
    }
    if (ImGui::SliderFloat("SFX", &mSFXVolume, 0.0f, 1.0f)) {
        audio->SetGroupVolume(ESoundGroup::SFX, mSFXVolume);
    }

    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> AudioDemoScene::InstantiateCopy() {
    return std::make_shared<AudioDemoScene>(*this);
}
