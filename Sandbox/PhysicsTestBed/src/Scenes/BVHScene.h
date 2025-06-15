#pragma once
#include "Game/Scene.h"
#include "Physics/Collision.h"

class BVHScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdated() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InsatiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    float mAreaSize        = 500.f;
    bool mPointColliding   = false;
    bool mAABBColliding    = false;
    bool mRayCastColliding = false;
    float mRayAngle        = 0;
    Umbra::Math::Vector2f mBoxSize;
};
