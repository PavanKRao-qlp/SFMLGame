#include "SimpleGameInstance.h"

#include "Diag/Logger.h"
#include "Game/SceneManager.h"
#include "GameStates.h"
#include "SimpleScene.h"

SimpleGameInstance::SimpleGameInstance() {}

SimpleGameInstance::~SimpleGameInstance() {}

void SimpleGameInstance::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> NewScene = std::make_shared<SimpleScene>();
    GetSceneManager().AddScene("Scene0", NewScene);
    GetSceneManager().GoToScene(NewScene);
}

void SimpleGameInstance::ShutDown() {}

Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication() {
    return std::make_shared<SimpleGameInstance>();
}
