#pragma once
#include "Umbra.h"
#include "FSM/FSM.h"
#include "SimpleGameInstance.h"

enum class GameplayStateID : Umbra::int8
{
    MAIN_MENU = 1,
    GAME = 2,
    GAME_OVER = 3
};

class GameplayState : public Umbra::IFSMState
{
private:
    Umbra::EntityID ship;
    void SpawnBG();
    void SpawnShip();
    void SpawnPlayerBullet();
    class Umbra::FiniteStateMachine *mGameplayState;
    float shootCooldown = 0;

public:
    GameplayState(/* args */);
    ~GameplayState();

    class SimpleGameInstance *mGameInstance;

protected:
    // Implementing the IFSMState interface

    void OnEnter() override;
    void OnUpdate() override;
    void OnExit() override;
};
