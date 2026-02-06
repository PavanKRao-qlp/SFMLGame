#pragma once
#include "Game/Scene.h"

class PointLineSegmentScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    Umbra::Math::Vector2f mPointA;
    Umbra::Math::Vector2f mPointB;
    float mDistance      = 0;
    float mOrthoDistance = 0;
    bool bOnLine         = false;
};
