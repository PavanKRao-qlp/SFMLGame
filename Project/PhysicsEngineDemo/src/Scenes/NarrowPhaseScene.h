#pragma once
#include "Game/Scene.h"
#include "Service/Physics/PhysicsHandle.h"

class NarrowPhaseScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    void DrawColliderOutlines();
    void DrawBoundingVolumes();
    void DrawCollisionInfo();

    void MovePlayerBox();

    Umbra::Vector<Umbra::EntityID> mEntities;
    Umbra::EntityID mPlayerBox = 0;
    float mPlayerSpeed         = 10.0f;
    bool bShowBoundingVolumes  = false;
    bool bShowContactPoints    = false;
    bool bShowContactNormals   = false;
    bool bShowPenetration      = false;
    int mCollisionCount        = 0;
    int mContactPointCount     = 0;
};
