#pragma once
#include "Game/Scene.h"

class SimpleScene : public Umbra::Scene
{
public:
    virtual void Initialize() override;
    virtual void OnUpdate() override;

    void OnBeginPlay() override;
};