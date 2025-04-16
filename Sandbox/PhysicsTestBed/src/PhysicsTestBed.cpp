#include "PhysicsTestBed.h"

#include "Scenes/CircleAABBScene.h"
#include "Scenes/CircleLineSegmentScene.h"
#include "Scenes/CircleOBBScene.h"
#include "Scenes/LandingScene.h"
#include "Scenes/PointLineSegmentScene.h"
#include "Scenes/RaycastAABB.h"
#include "Scenes/RaycastCircle.h"
#include "Scenes/RaycastLineSegment.h"
#include "Scenes/RaycastOBB.h"
#include "Scenes/SATScene.h"
#include "Scenes/TrianglePointScene.h"

PhysicsTestBed::PhysicsTestBed(/* args */) {}

PhysicsTestBed::~PhysicsTestBed() {
    sf::ConvexShape shape;
}

inline void PhysicsTestBed::Initialize() {
    Umbra::SharedPtr<Umbra::Scene> mainMenu               = std::make_shared<LandingScene>();
    Umbra::SharedPtr<Umbra::Scene> pointLineSegmentScene  = std::make_shared<PointLineSegmentScene>();
    Umbra::SharedPtr<Umbra::Scene> circleLineSegmentScene = std::make_shared<CircleLineSegmentScene>();
    Umbra::SharedPtr<Umbra::Scene> circleAABBScene        = std::make_shared<CircleAABBScene>();
    Umbra::SharedPtr<Umbra::Scene> circleOBBScene         = std::make_shared<CircleOBBScene>();
    Umbra::SharedPtr<Umbra::Scene> trianglePointScene     = std::make_shared<TrianglePointScene>();
    Umbra::SharedPtr<Umbra::Scene> raycastCircle          = std::make_shared<RaycastCircle>();
    Umbra::SharedPtr<Umbra::Scene> raycastLineSegment     = std::make_shared<RaycastLineSegment>();
    Umbra::SharedPtr<Umbra::Scene> raycastAABB            = std::make_shared<RaycastAABB>();
    Umbra::SharedPtr<Umbra::Scene> raycastOBB             = std::make_shared<RaycastOBB>();
    Umbra::SharedPtr<Umbra::Scene> satScene               = std::make_shared<SATScene>();
    GetSceneManger().AddScene("Scene0", mainMenu);
    GetSceneManger().AddScene("Scene1", pointLineSegmentScene);
    GetSceneManger().AddScene("Scene2", circleLineSegmentScene);
    GetSceneManger().AddScene("Scene3", circleAABBScene);
    GetSceneManger().AddScene("Scene4", circleOBBScene);
    GetSceneManger().AddScene("Scene5", trianglePointScene);
    GetSceneManger().AddScene("Scene6", raycastCircle);
    GetSceneManger().AddScene("Scene7", raycastLineSegment);
    GetSceneManger().AddScene("Scene8", raycastAABB);
    GetSceneManger().AddScene("Scene9", raycastOBB);
    GetSceneManger().AddScene("Scene10", satScene);
    GetSceneManger().GoToScene(mainMenu);
}

inline void PhysicsTestBed::ShutDown() {}
