#pragma once
#include "Game/IGameInstance.h"

class AudioTestBed : public Umbra::IGameInstance {
public:
    AudioTestBed();
    ~AudioTestBed();
    virtual void Initialize() override;
    virtual void ShutDown() override;
};

Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication() {
    return std::make_shared<AudioTestBed>();
}
