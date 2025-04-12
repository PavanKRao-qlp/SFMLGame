#include "PhysicsTestBed.h"

#include "Scenes/CircleAABBScene.h"
#include "Scenes/CircleLineSegmentScene.h"
#include "Scenes/CircleOBBScene.h"
#include "Scenes/LandingScene.h"
#include "Scenes/PointLineSegmentScene.h"
#include "Scenes/TrianglePointScene.h"

PhysicsTestBed::PhysicsTestBed(/* args */) {}

PhysicsTestBed::~PhysicsTestBed() {}

inline void PhysicsTestBed::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> mainMenu               = std::make_shared<LandingScene>();
    Umbra::SharedPtr<Umbra::Scene> pointLineSegmentScene  = std::make_shared<PointLineSegmentScene>();
    Umbra::SharedPtr<Umbra::Scene> circleLineSegmentScene = std::make_shared<CircleLineSegmentScene>();
    Umbra::SharedPtr<Umbra::Scene> circleAABBScene        = std::make_shared<CircleAABBScene>();
    Umbra::SharedPtr<Umbra::Scene> circleOBBScene         = std::make_shared<CircleOBBScene>();
    Umbra::SharedPtr<Umbra::Scene> trianglePointScene     = std::make_shared<TrianglePointScene>();
    GetSceneManger().AddScene("Scene0", mainMenu);
    GetSceneManger().AddScene("Scene1", pointLineSegmentScene);
    GetSceneManger().AddScene("Scene2", circleLineSegmentScene);
    GetSceneManger().AddScene("Scene3", circleAABBScene);
    GetSceneManger().AddScene("Scene4", circleOBBScene);
    GetSceneManger().AddScene("Scene5", trianglePointScene);
    GetSceneManger().GoToScene(mainMenu);
}

inline void PhysicsTestBed::ShutDown() {}
