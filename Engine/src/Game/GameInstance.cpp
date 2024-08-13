#include "Game/GameInstance.h"

bool GameInstance::Initialize()
{
    mCurrentGameWorld = new GameWorld();
    return true;
}

void GameInstance::ShutDown()
{
    delete mCurrentGameWorld;
}

GameWorld *GameInstance::GetCurrentGameWorld()
{
    return mCurrentGameWorld;
}

GameInstance::~GameInstance()
{
}