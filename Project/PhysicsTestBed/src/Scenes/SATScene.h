#pragma once
#include "Game/Scene.h"
#include "Math/Ray.h"
#include "Math/Vector.h"
#include "Physics/Collision.h"

class SATScene : public Umbra::Scene {

    enum ShapeType {
        Box,
        Polygon,
        Circle
    };

public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    Umbra::Vector<Umbra::Math::Vector2f> GetRandomPolygon(int _edges, int _sizeRadius);
    Umbra::EntityID mEntityA = Umbra::MAX_ENTITY;
    Umbra::EntityID mEntityB = Umbra::MAX_ENTITY;
    ShapeType mShapeA        = ShapeType::Box;
    ShapeType mShapeB        = ShapeType::Box;
    Umbra::Math::Polygon mPolygonA;
    Umbra::Math::Polygon mPolygonB;
    float mRadius;
    float mDistance;
    float mAngle           = 0;
    bool bShowProjections  = false;
    bool bDrawContactPoint = false;

    Umbra::SharedPtr<Umbra::CollisionDetector> mCollisionDetector;
};
