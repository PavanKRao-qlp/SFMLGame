#pragma once
#include "Game/Scene.h"

class CircleLineSegmentScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdated() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InsatiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    Umbra::Math::Vector2f mPointA;
    Umbra::Math::Vector2f mPointB;
    float mRadius        = 10;
    float mDistance      = 0;
    float mOrthoDistance = 0;
};
