#pragma once
#include "Game/Scene.h"

class SimpleScene : public Umbra::Scene
{
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdated() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InsatiateCopy() override;
    void OnBeginPlay() override;

private:
};