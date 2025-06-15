
#pragma once
#include "Game/Scene.h"
#include "Math/Triangle.h"

class TrianglePointScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdated() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InsatiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    Umbra::Math::Vector2f mPoint;
    Umbra::Math::Vector2f mSize;
    Umbra::Math::Triangle mTriangle;
    float mRadius         = 10;
    float mDistance       = 0;
    float mDistanceToProj = 0;
    float mOrthoDistance  = 0;
};
