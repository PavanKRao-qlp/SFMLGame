#include "LDtkViewerGameInstance.h"

#include "Game/SceneManager.h"
#include "LDtkViewerScene.h"

LDtkViewerGameInstance::LDtkViewerGameInstance() {}
LDtkViewerGameInstance::~LDtkViewerGameInstance() {}

void LDtkViewerGameInstance::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> scene =
        std::make_shared<LDtkViewerScene>("Asset/world.ldtk");
    GetSceneManager().AddScene("LDtkViewer", scene);
    GetSceneManager().GoToScene(scene);
}

void LDtkViewerGameInstance::ShutDown() {}

Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication() {
    return std::make_shared<LDtkViewerGameInstance>();
}
