#include "PhysicsEngineDemo.h"

#include "Scenes/ForceAndTorqueScene.h"
#include "Scenes/MainScene.h"
#include "Scenes/ParticleIntegrationScene.h"

PhysicsEngineDemo::PhysicsEngineDemo() {}

PhysicsEngineDemo::~PhysicsEngineDemo() {}

void PhysicsEngineDemo::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> mainScene                = std::make_shared<MainScene>();
    Umbra::SharedPtr<Umbra::Scene> particleIntegrationScene = std::make_shared<ParticleIntegrationScene>();
    Umbra::SharedPtr<Umbra::Scene> forceAndTorqueScene      = std::make_shared<ForceAndTorqueScene>();

    GetSceneManager().AddScene("MainScene", mainScene);
    GetSceneManager().AddScene("ParticleIntegration", particleIntegrationScene);
    GetSceneManager().AddScene("ForceAndTorqueScene", forceAndTorqueScene);

    GetSceneManager().GoToScene(mainScene);
}

void PhysicsEngineDemo::ShutDown() {}
