#pragma once
#include "Core/Clock.h"
#include "Core/Random.h"
#include "ECS/Component.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Components/Rigidbody.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/CollisionBox.h"
#include "Test.h"

class EnemySpawnSystem : public Umbra::System
{
public:
    inline EnemySpawnSystem(Umbra::EntityID _PlayerShipRef) : Umbra::System(new Umbra::ECView()), mPlayerShipRef(_PlayerShipRef)
    {
        mSpawnCoolDown = mTimeToSpawn;
        TestVec();
    };
    inline ~EnemySpawnSystem() {};
    inline void Update() override
    {
        if (mSpawnCoolDown > 0)
        {
            mSpawnCoolDown -= Umbra::EngineTime::GetDeltaTime();
        }
        else
        {
            SpawnEnemyShip();
            mSpawnCoolDown = mTimeToSpawn;
        }
    }

private:
    inline void SpawnEnemyShip()
    {
        Umbra::TransformComponent *playerShipTransform = mView->ecsRegister->GetComponent<Umbra::TransformComponent>(mPlayerShipRef);
        if (playerShipTransform != nullptr)
        {
            float angle = Umbra::Random::GetRandom() * Umbra::Math::PI * 2;
            float spawnDistance = 400;
            Umbra::Math::Vector2f randDir = Umbra::Math::Vector2f(Umbra::Math::Cos(angle), Umbra::Math::Sin(angle));
            Umbra::Math::Vector2f spawnPos = playerShipTransform->Position + (randDir * spawnDistance);
            Umbra::Math::Vector2f toShipDir = (playerShipTransform->Position - spawnPos).GetNormalized();

            float angleToShip = Umbra::Math::RadianToDegree(Umbra::Math::Atan2(toShipDir.y, toShipDir.x)) + 90;
            Umbra::EntityID enemyShip = mView->ecsRegister->CreateEntity();
            mView->ecsRegister->AddComponent<Umbra::TransformComponent>(enemyShip, Umbra::TransformComponent(spawnPos, Umbra::Math::Vector2f(32, 32), angleToShip));
            mView->ecsRegister->AddComponent<Umbra::SpriteComponent>(enemyShip, Umbra::AssetManager::getInstance()->GetTexture("Asset/Texture/T_ship_0005.png"));
            Umbra::RigidBodyComponent enemyShipRB;
            enemyShipRB.Velocity = toShipDir * 50;
            mView->ecsRegister->AddComponent<Umbra::RigidBodyComponent>(enemyShip, enemyShipRB);
            mView->ecsRegister->AddComponent<Umbra::CollisionBoxComponent>(enemyShip, Umbra::Math::Bounds2D(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(40, 40)));
            // Umbra::Debug::DrawBox()
            //  Umbra::Math::
        }
    };
    float mSpawnCoolDown = 0;
    float mTimeToSpawn = 2.5f;
    Umbra::EntityID mPlayerShipRef;
};