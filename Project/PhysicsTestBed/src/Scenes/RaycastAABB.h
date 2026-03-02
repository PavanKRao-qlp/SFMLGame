
#pragma once
#include "Game/Scene.h"
#include "Math/Ray.h"
#include "Math/Vector.h"

class RaycastAABB : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;


private:
    Umbra::Math::Vector2f mPoint;
    Umbra::Math::Vector2f mSize;
    float mRadius;
    Umbra::Math::Ray2D mRay;
    float mAngle = 0;
};
