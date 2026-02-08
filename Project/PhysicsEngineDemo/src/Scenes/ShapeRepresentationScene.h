#pragma once
#include "Game/Scene.h"
#include "Service/Physics/PhysicsHandle.h"

class ShapeRepresentationScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    Umbra::Vector<Umbra::EntityID> mEntities;
};
