#include "SimpleGameInstance.h"
#include "Diag/Logger.h"
#include "GameStates.h"
#include "SimpleScene.h"
#include "Game/SceneManager.h"

SimpleGameInstance::SimpleGameInstance()
{
}

SimpleGameInstance::~SimpleGameInstance()
{
}

void SimpleGameInstance::Initialize()
{
    Umbra::SharedPtr<Umbra::Scene> NewScene = std::make_shared<SimpleScene>();
    GetSceneManger()->AddScene(NewScene);
    GetSceneManger()->GoToScene(NewScene);
}

void SimpleGameInstance::ShutDown()
{
}

Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication()
{
    return std::make_shared<SimpleGameInstance>();
}