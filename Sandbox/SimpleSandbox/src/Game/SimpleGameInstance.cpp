#include "SimpleGameInstance.h"
#include "Diag/Logger.h"
#include "GameStates.h"

SimpleGameInstance::SimpleGameInstance()
{
}

SimpleGameInstance::~SimpleGameInstance()
{
}

void SimpleGameInstance::Initialize()
{
    Logger::Log(LogType::Verbose, "SimpleGameInstance Initialize!");
    mGameplayState = new Umbra::FiniteStateMachine();
    mGameplayState->AddState(static_cast<int>(GameplayStateID::MAIN_MENU), new MenuState(this));
    mGameplayState->AddState(static_cast<int>(GameplayStateID::GAME), new GameplayState(this));
}

void SimpleGameInstance::OnBeginPlay()
{
    Logger::Log(LogType::Verbose, "SimpleGameInstance OnBeginPlay!");
    mGameplayState->GoToState(static_cast<int>(GameplayStateID::MAIN_MENU));
}

void SimpleGameInstance::OnEndPlay()
{
    delete mGameplayState;
    Logger::Log(LogType::Verbose, "SimpleGameInstance OnEndPlay!");
}

void SimpleGameInstance::OnUpdate(float dt)
{
    mGameplayState->GetCurrentState()->OnUpdate();
}
