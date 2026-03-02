#pragma once
#include "Game/Scene.h"
#include "Math/Ray.h"
#include "Math/Vector.h"

class RaycastOBB : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    Umbra::EntityID mBoxEntity = Umbra::MAX_ENTITY;
    float mRadius;
    float mDistance;
    Umbra::Math::Ray2D mRay;
    float mAngle = 0;
};
