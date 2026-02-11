#pragma once
#include "Game/Scene.h"
#include "Service/Physics/PhysicsHandle.h"

class CollisionFilterCCDScene : public Umbra::Scene {
public:
    virtual void Initialize() override;
    virtual void OnFixedUpdate() override;
    virtual void OnUpdate() override;
    virtual Umbra::SharedPtr<Umbra::Scene> InstantiateCopy() override;
    void OnBeginPlay() override;
    void OnEndPlay() override;

public:
    // Collision filter category bits
    static constexpr uint16_t CATEGORY_DEFAULT = 0x0001;
    static constexpr uint16_t CATEGORY_RED     = 0x0002;
    static constexpr uint16_t CATEGORY_GREEN   = 0x0004;
    static constexpr uint16_t CATEGORY_BLUE    = 0x0008;
    static constexpr uint16_t CATEGORY_BULLET  = 0x0010;

    enum class EFilterGroup { Red, Green, Blue };

private:
    void DrawColliderOutlines();
    void SpawnBullet();

    struct EntityInfo {
        Umbra::EntityID Entity;
        EFilterGroup Group;
    };

    Umbra::Vector<EntityInfo> mDynamicEntities;
    Umbra::Vector<Umbra::EntityID> mStaticEntities;
    Umbra::Vector<Umbra::EntityID> mBulletEntities;

    bool bShowColliders       = true;
    bool bRedCollidesGreen    = false;
    bool bRedCollidesBlue     = false;
    bool bGreenCollidesBlue   = false;
    bool bBulletCCDEnabled    = true;
    float mBulletSpeed        = 2000.0f;
};
