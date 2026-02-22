#include "MultiThreadingApp.h"
#include "Game/SceneManager.h"
#include "Scenes/ThreadDemoScene.h"

void MultiThreadingApp::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> scene = std::make_shared<ThreadDemoScene>();
    GetSceneManager().AddScene("ThreadDemo", scene);
    GetSceneManager().GoToScene(scene);
}

void MultiThreadingApp::ShutDown() {}

Umbra::SharedPtr<Umbra::IGameInstance> CreateApplication() {
    return std::make_shared<MultiThreadingApp>();
}
