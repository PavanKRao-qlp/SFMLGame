#include "PhysicsEngineDemo.h"

#include "Scenes/MainScene.h"

PhysicsEngineDemo::PhysicsEngineDemo() {}

PhysicsEngineDemo::~PhysicsEngineDemo() {}

void PhysicsEngineDemo::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> mainScene = std::make_shared<MainScene>();
    GetSceneManager().AddScene("MainScene", mainScene);
    GetSceneManager().GoToScene(mainScene);
}

void PhysicsEngineDemo::ShutDown() {}
