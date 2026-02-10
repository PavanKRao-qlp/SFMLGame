#pragma once
#include "Game/Scene.h"
#include "Service/Physics/PhysicsHandle.h"

class BroadphaseTreeScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    void DrawColliderOutlines();
    void DrawTightAABBs();
    void DrawFatAABBs();
    void DrawTreeInternalNodes();
    void MovePlayer();
    void SpawnRandomBody();

    Umbra::Vector<Umbra::EntityID> mEntities;
    Umbra::EntityID mPlayerEntity = 0;
    float mPlayerSpeed            = 10.0f;

    bool bShowColliders      = true;
    bool bShowTightAABBs     = false;
    bool bShowFatAABBs       = true;
    bool bShowTreeNodes      = true;
};
