#include "AudioTestBed.h"

#include "Scenes/AudioDemoScene.h"

AudioTestBed::AudioTestBed() {}

AudioTestBed::~AudioTestBed() {}

inline void AudioTestBed::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> audioDemo = std::make_shared<AudioDemoScene>();
    GetSceneManager().AddScene("AudioDemo", audioDemo);
    GetSceneManager().GoToScene(audioDemo);
}

inline void AudioTestBed::ShutDown() {}
