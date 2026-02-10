#include "PhysicsEngineDemo.h"

#include "Scenes/BoundingVolumeScene.h"
#include "Scenes/ForceAndTorqueScene.h"
#include "Scenes/MainScene.h"
#include "Scenes/ParticleIntegrationScene.h"
#include "Scenes/NarrowPhaseScene.h"
#include "Scenes/ShapeRepresentationScene.h"
#include "Scenes/BroadphaseTreeScene.h"

PhysicsEngineDemo::PhysicsEngineDemo() {}

PhysicsEngineDemo::~PhysicsEngineDemo() {}

void PhysicsEngineDemo::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> mainScene                = std::make_shared<MainScene>();
    Umbra::SharedPtr<Umbra::Scene> particleIntegrationScene = std::make_shared<ParticleIntegrationScene>();
    Umbra::SharedPtr<Umbra::Scene> forceAndTorqueScene      = std::make_shared<ForceAndTorqueScene>();
    Umbra::SharedPtr<Umbra::Scene> shapeRepresentationScene = std::make_shared<ShapeRepresentationScene>();
    Umbra::SharedPtr<Umbra::Scene> boundingVolumeScene      = std::make_shared<BoundingVolumeScene>();
    Umbra::SharedPtr<Umbra::Scene> narrowPhaseScene         = std::make_shared<NarrowPhaseScene>();
    Umbra::SharedPtr<Umbra::Scene> broadphaseTreeScene     = std::make_shared<BroadphaseTreeScene>();

    GetSceneManager().AddScene("MainScene", mainScene);
    GetSceneManager().AddScene("ParticleIntegration", particleIntegrationScene);
    GetSceneManager().AddScene("ForceAndTorque", forceAndTorqueScene);
    GetSceneManager().AddScene("ShapeRepresentation", shapeRepresentationScene);
    GetSceneManager().AddScene("BoundingVolume", boundingVolumeScene);
    GetSceneManager().AddScene("NarrowPhase", narrowPhaseScene);
    GetSceneManager().AddScene("BroadphaseTree", broadphaseTreeScene);

    GetSceneManager().GoToScene(mainScene);
}

void PhysicsEngineDemo::ShutDown() {}
