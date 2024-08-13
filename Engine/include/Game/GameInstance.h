#pragma once
#include "Core/Singleton.h"
#include "Game/GameWorld.h"

class GameInstance : public Singleton<GameInstance>
{
public:
    ~GameInstance();
    bool Initialize();
    void ShutDown();
    GameWorld *GetCurrentGameWorld();

private:
    GameWorld *mCurrentGameWorld;
};