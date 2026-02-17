#pragma once
#include "Game/Scene.h"
#include "Service/Physics/PhysicsHandle.h"

class MainScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    void SetupArena();
    void SetupSpringChain();
    void SetupWindmill();
    void SetupNewtonCradle();
    void SetupRamp();
    void SetupTower();
    void SetupLooseBodies();
    void CreateConstraints();
    void DrawColliderOutlines();
    void DrawDebugVisuals();
    void CleanupPlayground();

    Umbra::Vector<Umbra::EntityID> mEntities;
    Umbra::Vector<Umbra::ConstraintHandle> mConstraints;
    bool bNeedsConstraintSetup = false;
    bool bNeedsInitialVelocity = false;

    // Spring chain entity indices (into mEntities)
    static constexpr int SPRING_CHAIN_START = 0;
    static constexpr int SPRING_CHAIN_COUNT = 5;
    Umbra::Math::Vector2f mSpringWorldAnchor = {-200.0f, 220.0f};

    // Windmill entity indices
    int mWindmillPivotIdx = -1;
    int mWindmillArmIdx   = -1;

    // Newton's cradle entity indices
    int mCradleBarIdx        = -1;
    int mCradleBallStartIdx  = -1;
    static constexpr int CRADLE_BALL_COUNT = 5;

    // Loose body indices (for initial velocities)
    int mLooseBodyStartIdx = -1;
    int mLooseBodyCount    = 0;
};
