#pragma once
#include "Game/IGameInstance.h"

class SimpleGameInstance : public D2D::IGameInstance
{
private:
    /* data */
public:
    SimpleGameInstance();
    ~SimpleGameInstance();
    virtual void Initialize() override;
    virtual void OnBeginPlay() override;
    virtual void OnEndPlay() override;
    virtual void OnUpdate(float dt) override;
};
