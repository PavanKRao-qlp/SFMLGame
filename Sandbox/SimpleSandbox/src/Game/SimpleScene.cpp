#include "SimpleScene.h"
#include "Umbra.h"
#include "Input/Input.h"
#include "Game/IGameInstance.h"
#include "ECS/Components/PhysicsBodyComponent.h"
#include "PointObjectSpawnSystem.h"

void SimpleScene::Initialize()
{
    UMBRA_LOG_INFO("SimpleScene Initialized !!");
    ObjectSpawner = std::make_shared<PointObjectSpawnSystem>();
    GetWorld()->AddSystem(Umbra::ESystemPhase::FrameStart, 1, ObjectSpawner);
    //  AddSystem();
}

void SimpleScene::OnFixedUpdated()
{
}

void SimpleScene::OnUpdate()
{
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape))
    {
        GetGameInstance().QuitApplication();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Space))
    {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Right))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.x += 10 * Umbra::EngineTime::GetDeltaTime();
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Left))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.x -= 10 * Umbra::EngineTime::GetDeltaTime();
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Up))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.y += 10 * Umbra::EngineTime::GetDeltaTime();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Down))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.y -= 10 * Umbra::EngineTime::GetDeltaTime();
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Q))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
        {
            float orthographic = GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->GetOrthographicSize();
            orthographic += 10 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::E))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
        {
            float orthographic = GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->GetOrthographicSize();
            orthographic -= 10 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
}

Umbra::SharedPtr<Umbra::Scene> SimpleScene::InsatiateCopy()
{
    return std::make_shared<SimpleScene>(*this);
}

void SimpleScene::OnBeginPlay()
{
    // Umbra::EntityID entity = GetWorld()->CreateEntity();
    // GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Red);
    // GetWorld()->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
    //                                                                 Umbra::Math::Vector2f(0, 50),
    //                                                                 Umbra::Math::Vector2f(10, 10)));
    // Umbra::PhysicsBodyComponent physicsBodyComponent = Umbra::PhysicsBodyComponent();
    // physicsBodyComponent.SetMass(1);
    // physicsBodyComponent.mVelocity = Umbra::Math::Vector2f(10, 0);
    // physicsBodyComponent.mAcceleration = Umbra::Math::Vector2f(0, -9.8f);
    // GetWorld()->AddComponent<Umbra::PhysicsBodyComponent>(entity, physicsBodyComponent);

    // entity = GetWorld()->CreateEntity();
    // GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Blue);
    // GetWorld()->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
    //                                                                 Umbra::Math::Vector2f(0, 50),
    //                                                                 Umbra::Math::Vector2f(50, 50)));
    // entity = GetWorld()->CreateEntity();
    // GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Green);
    // GetWorld()->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
    //                                                                 Umbra::Math::Vector2f(0, -50),
    //                                                                 Umbra::Math::Vector2f(50, 50)));
    // GetWorld()->Create
    // //   GetWorld()->Create
}
