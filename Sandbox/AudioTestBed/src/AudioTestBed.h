#pragma once
#include "Game/IGameInstance.h"
class AudioTestBed : public Umbra::IGameInstance {
private:
    /* data */
public:
    AudioTestBed(/* args */);
    ~AudioTestBed();
    virtual void Initialize() override;
    virtual void ShutDown() override;
};

Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication() {
    return std::make_shared<AudioTestBed>();
}
