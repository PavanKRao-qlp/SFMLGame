#include "SimpleGameInstance.h"
#include "Input/Input.h"
#include "ECS/ECSRegister.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Components/Rigidbody.h"
#include "Math/CollisionSystem.h"
#include "Asset/AssetManager.h"
#include "Asset/Texture.h"
#include "Diag/Logger.h"
#include "EnemySpawnSystem.h"

SimpleGameInstance::SimpleGameInstance()
{
}

SimpleGameInstance::~SimpleGameInstance()
{
}

void SimpleGameInstance::Initialize()
{
    Logger::Log(LogType::Verbose, "SimpleGameInstance Initialize!");
    SpawnBG();
    SpawnShip();
    mWorldRegister->AddSystem(new EnemySpawnSystem(ship));
}

void SimpleGameInstance::OnBeginPlay()
{
    Logger::Log(LogType::Verbose, "SimpleGameInstance OnBeginPlay!");
}

void SimpleGameInstance::OnEndPlay()
{
    Logger::Log(LogType::Verbose, "SimpleGameInstance OnEndPlay!");
}

void SimpleGameInstance::OnUpdate(float dt)
{
    Umbra::TransformComponent *shipTransform = mWorldRegister->GetComponent<Umbra::TransformComponent>(ship);
    if (shipTransform != nullptr)
    {
        Umbra::Math::Vector2f worldPos2D = GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
        Umbra::Math::Vector2f mouseDir = (worldPos2D - shipTransform->Position).GetNormalized();
        float angle = Umbra::Math::RadianToDegree(Umbra::Math::Atan2(mouseDir.y, mouseDir.x));
        shipTransform->Angle = (angle) + 90;
    }
    if (shootCooldown > 0)
        shootCooldown -= dt;
    if (Umbra::Input::GetMouseButtonDown(Umbra::Mouse::Left) && shootCooldown <= 0)
    {
        shootCooldown = 0.15f;
        SpawnPlayerBullet();
    };

    Umbra::Vector<Umbra::Collision::CollisionResponse> OutResponses;
    if (Umbra::Collision::QueryCollisionsForTag("Enemy", OutResponses))
    {
        for (Umbra::Collision::CollisionResponse response : OutResponses)
        {
            if ((mWorldRegister->IsTag(response.mEntityA, "Enemy") && mWorldRegister->IsTag(response.mEntityB, "PlayerBullet")) || (mWorldRegister->IsTag(response.mEntityB, "Enemy") && mWorldRegister->IsTag(response.mEntityA, "PlayerBullet")))
            {
                mWorldRegister->DestroyEntity(response.mEntityA);
                mWorldRegister->DestroyEntity(response.mEntityB);
            }
        }
    }

    if (Umbra::Collision::QueryCollisionsForEntityID(ship, OutResponses))
    {
        for (Umbra::Collision::CollisionResponse response : OutResponses)
        {
            if (response.mEntityA == ship && mWorldRegister->IsTag(response.mEntityB, "Enemy") || (mWorldRegister->IsTag(response.mEntityA, "Enemy") && response.mEntityB == ship))
            {
                mWorldRegister->DestroyEntity(response.mEntityA);
                mWorldRegister->DestroyEntity(response.mEntityB);
            }
        }
    }
}

void SimpleGameInstance::SpawnPlayerBullet()
{
    Umbra::TransformComponent *shipTransform = mWorldRegister->GetComponent<Umbra::TransformComponent>(ship);
    Umbra::Math::Vector2f bulletPos = shipTransform->Position + (shipTransform->GetForward() * 10);
    Umbra::EntityID bullet = mWorldRegister->CreateEntity();
    mWorldRegister->AddTag(bullet, "PlayerBullet");
    mWorldRegister->AddComponent<Umbra::TransformComponent>(bullet, Umbra::TransformComponent(shipTransform->Position, Umbra::Math::Vector2f(16, 16), shipTransform->Angle));
    mWorldRegister->AddComponent<Umbra::SpriteComponent>(bullet, Umbra::AssetManager::GetInstance()->GetTexture("Asset/Texture/T_Bullet.png"));
    mWorldRegister->AddComponent<Umbra::LifeTimeComponent>(bullet, Umbra::LifeTimeComponent(1.5f));
    mWorldRegister->AddComponent<Umbra::CollisionBoxComponent>(bullet, Umbra::CollisionBoxComponent(Umbra::Math::Bounds2D(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(20, 20))));
    Umbra::RigidBodyComponent bulletRB;
    bulletRB.Velocity = shipTransform->GetUp() * 150.f;
    mWorldRegister->AddComponent<Umbra::RigidBodyComponent>(bullet, bulletRB);
}

void SimpleGameInstance::SpawnShip()
{
    ship = mWorldRegister->CreateEntity();
    mWorldRegister->AddComponent<Umbra::TransformComponent>(ship, Umbra::TransformComponent(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(32, 32)));
    mWorldRegister->AddComponent<Umbra::SpriteComponent>(ship, Umbra::AssetManager::GetInstance()->GetTexture("Asset/Texture/T_ship_0000.png"));
    mWorldRegister->AddComponent<Umbra::CollisionBoxComponent>(ship, Umbra::CollisionBoxComponent(Umbra::Math::Bounds2D(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(20, 20))));
}

void SimpleGameInstance::SpawnBG()
{
    Umbra::EntityID sky = mWorldRegister->CreateEntity();
    mWorldRegister->AddComponent<Umbra::TransformComponent>(sky, Umbra::TransformComponent(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(400, 400)));
    mWorldRegister->AddComponent<Umbra::SpriteComponent>(sky, Umbra::AssetManager::GetInstance()->GetTexture("Asset/Texture/T__BgSpace.png"));
}