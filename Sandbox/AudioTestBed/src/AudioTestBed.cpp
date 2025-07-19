#include "AudioTestBed.h"

#include "Scenes/LandingScene.h"
#include "Scenes/SfxScene.h"

AudioTestBed::AudioTestBed() {}

AudioTestBed::~AudioTestBed() {}

void AudioTestBed::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> mainMenu = std::make_shared<LandingScene>();
    Umbra::SharedPtr<Umbra::Scene> sfxScene = std::make_shared<SfxScene>();
    GetSceneManger().AddScene("Scene0", mainMenu);
    GetSceneManger().AddScene("Scene1", sfxScene);
    GetSceneManger().GoToScene("Scene0");
}

void AudioTestBed::ShutDown() {}
