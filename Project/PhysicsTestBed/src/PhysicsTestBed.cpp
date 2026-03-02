#include "PhysicsTestBed.h"

#include "Scenes/BVHScene.h"
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
    Umbra::SharedPtr<Umbra::Scene> bvhScene               = std::make_shared<BVHScene>();
    GetSceneManager().AddScene("Scene0", mainMenu);
    GetSceneManager().AddScene("Scene1", pointLineSegmentScene);
    GetSceneManager().AddScene("Scene2", circleLineSegmentScene);
    GetSceneManager().AddScene("Scene3", circleAABBScene);
    GetSceneManager().AddScene("Scene4", circleOBBScene);
    GetSceneManager().AddScene("Scene5", trianglePointScene);
    GetSceneManager().AddScene("Scene6", raycastCircle);
    GetSceneManager().AddScene("Scene7", raycastLineSegment);
    GetSceneManager().AddScene("Scene8", raycastAABB);
    GetSceneManager().AddScene("Scene9", raycastOBB);
    GetSceneManager().AddScene("Scene10", satScene);
    GetSceneManager().AddScene("Scene11", bvhScene);
    GetSceneManager().GoToScene(mainMenu);
}

inline void PhysicsTestBed::ShutDown() {}
