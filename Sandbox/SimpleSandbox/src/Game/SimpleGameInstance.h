#pragma once
#include "Game/IGameInstance.h"

class SimpleGameInstance : public Umbra::IGameInstance
{
public:
    SimpleGameInstance();
    ~SimpleGameInstance();
    virtual void Initialize() override;
    virtual void OnBeginPlay() override;
    virtual void OnEndPlay() override;
    virtual void OnUpdate(float dt) override;

private:
    Umbra::EntityID ship;
};
