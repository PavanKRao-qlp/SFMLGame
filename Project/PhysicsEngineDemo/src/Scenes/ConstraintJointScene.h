#pragma once
#include "Game/Scene.h"
#include "Service/Physics/PhysicsHandle.h"

class ConstraintJointScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    void SetupEntities();
    void CreateConstraints();
    void CleanupScene();
    void DrawConstraintLines();
    void DrawColliderOutlines();

    Umbra::Vector<Umbra::EntityID> mEntities;
    Umbra::Vector<Umbra::ConstraintHandle> mConstraints;

    bool bNeedsConstraintSetup = false;

    // Spring body handle (for drawing line to world anchor)
    Umbra::BodyHandle mSpringBodyHandle;
    Umbra::Math::Vector2f mSpringWorldAnchor;

    // Distance constraint body handles (for drawing line between them)
    Umbra::BodyHandle mDistanceBodyA;
    Umbra::BodyHandle mDistanceBodyB;

    // Hinge constraint body handles
    Umbra::BodyHandle mHingeBodyA;
    Umbra::BodyHandle mHingeBodyB;

    // Constraint handles for runtime tweaking
    Umbra::ConstraintHandle mSpringHandle;
    Umbra::ConstraintHandle mDistanceHandle;
    Umbra::ConstraintHandle mHingeHandle;

    // Entity indices into mEntities for constraint creation
    // [0]=floor, [1]=springBody, [2]=distAnchor, [3]=distPendulum, [4]=hingeAnchor, [5]=hingeArm
    static constexpr int IDX_SPRING_BODY    = 1;
    static constexpr int IDX_DIST_ANCHOR    = 2;
    static constexpr int IDX_DIST_PENDULUM  = 3;
    static constexpr int IDX_HINGE_ANCHOR   = 4;
    static constexpr int IDX_HINGE_ARM      = 5;

    // Spring ImGui parameters
    float mSpringStiffness  = 80.0f;
    float mSpringDamping    = 4.0f;
    float mSpringRestLength = 50.0f;

    // Hinge ImGui parameters
    bool bHingeEnableLimits    = true;
    float mHingeLowerAngle     = -1.57f;
    float mHingeUpperAngle     = 1.57f;
    bool bHingeEnableMotor     = false;
    float mHingeMotorSpeed     = 2.0f;
    float mHingeMaxMotorTorque = 200.0f;

    bool bShowColliders = true;

    // Spring chain (mouse-attached)
    static constexpr int CHAIN_LINK_COUNT = 8;
    float mChainRadius                    = 8.0f;
    float mChainSpacing                   = 25.0f;
    float mChainStiffness                 = 300.0f;
    float mChainDamping                   = 8.0f;
    float mChainRestLength                = 20.0f;
    int mChainStartIdx                    = -1; // index into mEntities where chain head starts
    Umbra::Vector<Umbra::BodyHandle> mChainBodyHandles;
};
