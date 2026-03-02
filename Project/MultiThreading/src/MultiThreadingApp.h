#pragma once
#include "Game/IGameInstance.h"

class MultiThreadingApp : public Umbra::IGameInstance {
public:
    MultiThreadingApp() = default;
    virtual void Initialize() override;
    virtual void ShutDown() override;
};
