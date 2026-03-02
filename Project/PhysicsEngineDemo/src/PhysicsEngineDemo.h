#pragma once
#include "Game/IGameInstance.h"

class PhysicsEngineDemo : public Umbra::IGameInstance {
public:
    PhysicsEngineDemo();
    ~PhysicsEngineDemo();
    virtual void Initialize() override;
    virtual void ShutDown() override;
};

Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication() {
    return std::make_shared<PhysicsEngineDemo>();
}
