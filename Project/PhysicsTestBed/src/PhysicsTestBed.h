#pragma once
#include "Game/IGameInstance.h"
class PhysicsTestBed : public Umbra::IGameInstance {
private:
    /* data */
public:
    PhysicsTestBed(/* args */);
    ~PhysicsTestBed();
    virtual void Initialize() override;
    virtual void ShutDown() override;
};

Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication() {
    return std::make_shared<PhysicsTestBed>();
}
