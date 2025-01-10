#pragma once
#include "Game/IGameInstance.h"
#include "FSM/FSM.h"

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
    void SpawnBG();
    void SpawnShip();
    void SpawnPlayerBullet();
    class Umbra::FiniteStateMachine *mGameplayState;
    float shootCooldown = 0;
};
