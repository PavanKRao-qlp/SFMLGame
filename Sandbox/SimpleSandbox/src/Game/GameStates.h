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

class MenuState : public Umbra::IFSMState
{
public:
    MenuState(SimpleGameInstance *_gameInstance);
    ~MenuState();

protected:
    void OnEnter() override;
    void OnUpdate() override;
    void OnExit() override;

private:
    void SpawnBG();
    class SimpleGameInstance *mGameInstance;
};

class GameplayState : public Umbra::IFSMState
{

public:
    GameplayState(SimpleGameInstance *_gameInstance);
    ~GameplayState();

protected:
    void OnEnter() override;
    void OnUpdate() override;
    void OnExit() override;

private:
    Umbra::EntityID ship;
    void SpawnBG();
    void SpawnShip();
    void HandleShip();
    void SpawnPlayerBullet();
    float shootCooldown = 0;
    float ResetCooldown = 2.5f;
    class SimpleGameInstance *mGameInstance;
};
