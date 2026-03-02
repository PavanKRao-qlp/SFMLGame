#pragma once
#include "Game/Scene.h"
#include "Service/Audio/AudioHandle.h"

class AudioDemoScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnBeginPlay() override;
    virtual void OnEndPlay() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;

private:
    Umbra::SoundHandle mMusicHandle;
    float mMasterVolume = 1.0f;
    float mMusicVolume  = 1.0f;
    float mSFXVolume    = 1.0f;
    bool bMusicPlaying  = false;
};
