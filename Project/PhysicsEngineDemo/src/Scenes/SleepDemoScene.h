#pragma once
#include "Game/Scene.h"
#include "Service/Physics/PhysicsHandle.h"

class SleepDemoScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    void DrawColliderOutlines();
    void SpawnDynamicBody();
    void MovePlayer();
    void CountSleepStats(int& _awake, int& _sleeping);

    Umbra::Vector<Umbra::EntityID> mEntities;
    Umbra::EntityID mPlayerEntity = 0;
    float mPlayerSpeed            = 150.0f;
    bool bShowColliders           = true;

    float mLinearSleepThreshold  = 0.5f;
    float mAngularSleepThreshold = 0.2f;
    float mSleepTimeThreshold    = 0.5f;
};
