#include "GameStates.h"
#include "Input/Input.h"
#include "ECS/ECSRegister.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Components/Rigidbody.h"
#include "Math/CollisionSystem.h"
#include "Asset/AssetManager.h"
#include "EnemySpawnSystem.h"
#include "Asset/Texture.h"

GameplayState::GameplayState(SimpleGameInstance *_gameInstance) : mGameInstance(_gameInstance)
{
}

GameplayState::~GameplayState()
{
}

void GameplayState::OnEnter()
{
    SpawnBG();
    SpawnShip();
    mGameInstance->GetWorld()->GetRegister()->AddSystem(new EnemySpawnSystem(ship));
}

void GameplayState::OnExit()
{
    mGameInstance->GetWorld()->FlushWorld();
}

void GameplayState::OnUpdate()
{
    HandleShip();

    Umbra::Vector<Umbra::Collision::CollisionResponse> OutResponses;
    if (Umbra::Collision::QueryCollisionsForTag("Enemy", OutResponses))
    {
        for (Umbra::Collision::CollisionResponse response : OutResponses)
        {
            if ((mGameInstance->GetWorld()->GetRegister()->IsTag(response.mEntityA, "Enemy") && mGameInstance->GetWorld()->GetRegister()->IsTag(response.mEntityB, "PlayerBullet")) || (mGameInstance->GetWorld()->GetRegister()->IsTag(response.mEntityB, "Enemy") && mGameInstance->GetWorld()->GetRegister()->IsTag(response.mEntityA, "PlayerBullet")))
            {
                mGameInstance->GetWorld()->GetRegister()->DestroyEntity(response.mEntityA);
                mGameInstance->GetWorld()->GetRegister()->DestroyEntity(response.mEntityB);
            }
        }
    }

    if (Umbra::Collision::QueryCollisionsForEntityID(ship, OutResponses))
    {
        for (Umbra::Collision::CollisionResponse response : OutResponses)
        {
            if (response.mEntityA == ship && mGameInstance->GetWorld()->GetRegister()->IsTag(response.mEntityB, "Enemy") || (mGameInstance->GetWorld()->GetRegister()->IsTag(response.mEntityA, "Enemy") && response.mEntityB == ship))
            {
                mGameInstance->GetWorld()->GetRegister()->DestroyEntity(response.mEntityA);
                mGameInstance->GetWorld()->GetRegister()->DestroyEntity(response.mEntityB);
                mFSM->GoToState((int)GameplayStateID::MAIN_MENU);
            }
        }
    }
}

void GameplayState::HandleShip()
{
    Umbra::TransformComponent *shipTransform = mGameInstance->GetWorld()->GetRegister()->GetComponent<Umbra::TransformComponent>(ship);
    if (shipTransform != nullptr)
    {
        Umbra::Math::Vector2f worldPos2D = mGameInstance->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
        Umbra::Math::Vector2f mouseDir = (worldPos2D - shipTransform->Position).GetNormalized();
        float angle = Umbra::Math::RadianToDegree(Umbra::Math::Atan2(mouseDir.y, mouseDir.x));
        shipTransform->Angle = (angle) + 90;
    }
    if (shootCooldown > 0)
        shootCooldown -= Umbra::EngineTime::GetDeltaTime();
    if (Umbra::Input::GetMouseButtonDown(Umbra::Mouse::Left) && shootCooldown <= 0)
    {
        shootCooldown = 0.15f;
        SpawnPlayerBullet();
    };
}

void GameplayState::SpawnBG()
{
    Umbra::EntityID sky = mGameInstance->GetWorld()->GetRegister()->CreateEntity();
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::TransformComponent>(sky, Umbra::TransformComponent(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(400, 400)));
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::SpriteComponent>(sky, Umbra::AssetManager::GetInstance()->GetTexture("Asset/Texture/T__BgSpace.png"));
}

void GameplayState::SpawnShip()
{
    ship = mGameInstance->GetWorld()->GetRegister()->CreateEntity();
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::TransformComponent>(ship, Umbra::TransformComponent(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(32, 32)));
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::SpriteComponent>(ship, Umbra::AssetManager::GetInstance()->GetTexture("Asset/Texture/T_ship_0000.png"));
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::CollisionBoxComponent>(ship, Umbra::CollisionBoxComponent(Umbra::Math::Bounds2D(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(20, 20))));
}

void GameplayState::SpawnPlayerBullet()
{
    Umbra::TransformComponent *shipTransform = mGameInstance->GetWorld()->GetRegister()->GetComponent<Umbra::TransformComponent>(ship);
    Umbra::Math::Vector2f bulletPos = shipTransform->Position + (shipTransform->GetForward() * 10);
    Umbra::EntityID bullet = mGameInstance->GetWorld()->GetRegister()->CreateEntity();
    mGameInstance->GetWorld()->GetRegister()->AddTag(bullet, "PlayerBullet");
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::TransformComponent>(bullet, Umbra::TransformComponent(shipTransform->Position, Umbra::Math::Vector2f(16, 16), shipTransform->Angle));
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::SpriteComponent>(bullet, Umbra::AssetManager::GetInstance()->GetTexture("Asset/Texture/T_Bullet.png"));
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::LifeTimeComponent>(bullet, Umbra::LifeTimeComponent(1.5f));
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::CollisionBoxComponent>(bullet, Umbra::CollisionBoxComponent(Umbra::Math::Bounds2D(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(20, 20))));
    Umbra::RigidBodyComponent bulletRB;
    bulletRB.Velocity = shipTransform->GetUp() * 150.f;
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::RigidBodyComponent>(bullet, bulletRB);
}

MenuState::MenuState(SimpleGameInstance *_gameInstance)
{
    mGameInstance = _gameInstance;
}

MenuState::~MenuState()
{
}

void MenuState::OnEnter()
{
    SpawnBG();
}

void MenuState::OnUpdate()
{
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Keycode::Space))
    {
        mFSM->GoToState((int)GameplayStateID::GAME);
    }
}

void MenuState::OnExit()
{
    mGameInstance->GetWorld()->FlushWorld();
}

void MenuState::SpawnBG()
{
    Umbra::EntityID sky = mGameInstance->GetWorld()->GetRegister()->CreateEntity();
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::TransformComponent>(sky, Umbra::TransformComponent(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(400, 400)));
    mGameInstance->GetWorld()->GetRegister()->AddComponent<Umbra::SpriteComponent>(sky, Umbra::AssetManager::GetInstance()->GetTexture("Asset/Texture/T_MenuBG.png"));
}