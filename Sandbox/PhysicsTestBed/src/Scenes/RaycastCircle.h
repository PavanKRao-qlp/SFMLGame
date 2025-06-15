
#pragma once
#include "Game/Scene.h"
#include "Math/Ray.h"
#include "Math/Vector.h"

class RaycastCircle : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdated() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InsatiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    Umbra::Math::Vector2f mPoint;
    float mRadius;
    Umbra::Math::Ray2D mRay;
    float mAngle = 0;
};
