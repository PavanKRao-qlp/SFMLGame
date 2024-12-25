#include "SimpleGameInstance.h"
#include "Input/Input.h"
#include "ECS/ECSRegister.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Components/Rigidbody.h"
#include "Asset/AssetManager.h"
#include "Asset/Texture.h"
#include "Diag/Logger.h"

SimpleGameInstance::SimpleGameInstance()
{
}

SimpleGameInstance::~SimpleGameInstance()
{
}

void SimpleGameInstance::Initialize()
{
    Logger::Log(LogType::Verbose, "SimpleGameInstance Initialize!");

    Umbra::EntityID sky = mWorldRegister->CreateEntity();
    mWorldRegister->AddComponent<Umbra::TransformComponent>(sky, Umbra::TransformComponent(Umbra::Math::Vector2i(0, 0), Umbra::Math::Vector2i(400, 400)));
    mWorldRegister->AddComponent<Umbra::SpriteComponent>(sky, Umbra::AssetManager::getInstance()->GetTexture("Asset/Texture/T__BgSpace.png"));

    ship = mWorldRegister->CreateEntity();
    mWorldRegister->AddComponent<Umbra::TransformComponent>(ship, Umbra::TransformComponent(Umbra::Math::Vector2i(0, 0), Umbra::Math::Vector2i(32, 32)));
    mWorldRegister->AddComponent<Umbra::SpriteComponent>(ship, Umbra::AssetManager::getInstance()->GetTexture("Asset/Texture/T_ship_0000.png"));

    // mWorldRegister->AddComponent<Umbra::SpriteComponent>(ship, Umbra::AssetManager::getInstance()->GetTexture("Asset/Texture/T_Grid.png"));
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
        Umbra::Math::Vector2f mouseDir = hlslpp::normalize(worldPos2D - shipTransform->Position);
        float angle = Umbra::Math::RadianToDegree(Umbra::Math::Atan2(mouseDir.y, mouseDir.x));
        shipTransform->Angle = (angle) + 90;
    }
    if (Umbra::Input::GetMouseButtonDown(Umbra::Mouse::Left))
    {
        Umbra::TransformComponent *shipTransform = mWorldRegister->GetComponent<Umbra::TransformComponent>(ship);
        Umbra::Math::Vector2f bulletPos = shipTransform->Position + (shipTransform->GetForward() * 10);
        Umbra::EntityID bullet = mWorldRegister->CreateEntity();
        mWorldRegister->AddComponent<Umbra::TransformComponent>(bullet, Umbra::TransformComponent(shipTransform->Position, Umbra::Math::Vector2i(16, 16), shipTransform->Angle));
        mWorldRegister->AddComponent<Umbra::SpriteComponent>(bullet, Umbra::AssetManager::getInstance()->GetTexture("Asset/Texture/T_Bullet.png"));
        mWorldRegister->AddComponent<Umbra::LifeTimeComponent>(bullet, Umbra::LifeTimeComponent(1.5f));
        Umbra::RigidBodyComponent bulletRB;
        bulletRB.Velocity = shipTransform->GetUp() * 0.35f;
        mWorldRegister->AddComponent<Umbra::RigidBodyComponent>(bullet, bulletRB);
        // SpawnBullet();

        // mWorldRegister->AddComponent<Umbra::TransformComponent>(ballA, worldPos2D, Umbra::Math::Vector2i(10, 10));
        // mWorldRegister->AddComponent<Umbra::SpriteComponent>(ballA, sf::Color::Red);
        // mWorldRegister->AddComponent<Umbra::LifeTimeComponent>(ballA, 5.f);
    };
}

void SimpleGameInstance::SpawnBullet()
{
}
