#include "SimpleScene.h"
#include "Umbra.h"
#include "Input/Input.h"
#include "Game/IGameInstance.h"

void SimpleScene::Initialize()
{
    UMBRA_LOG_INFO("SimpleScene Initialized !!");
    // AddSystem();
}

void SimpleScene::OnFixedUpdated()
{
}

void SimpleScene::OnUpdate()
{
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape))
    {
        GetGameInstance()->QuitApplication();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Space))
    {
        auto a = GetGameInstance()->GetSceneManger();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Right))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.x += 10 * Umbra::EngineTime::GetDeltaTime();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Left))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.x -= 10 * Umbra::EngineTime::GetDeltaTime();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Up))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.y += 10 * Umbra::EngineTime::GetDeltaTime();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Down))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.y -= 10 * Umbra::EngineTime::GetDeltaTime();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Q))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
        {
            float orthographic = GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->GetOrthographicSize();
            orthographic += 10 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::E))
    {
        if (GetCameraEntity() != Umbra::MAX_ENTITY)
        {
            float orthographic = GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->GetOrthographicSize();
            orthographic -= 10 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
}

void SimpleScene::OnBeginPlay()
{
    Umbra::EntityID entity = GetWorld()->CreateEntity();
    GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Red);
    GetWorld()->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
                                                                    Umbra::Math::Vector2f(0, 0),
                                                                    Umbra::Math::Vector2f(50, 50)));
    entity = GetWorld()->CreateEntity();
    GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Blue);
    GetWorld()->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
                                                                    Umbra::Math::Vector2f(0, 50),
                                                                    Umbra::Math::Vector2f(50, 50)));
    entity = GetWorld()->CreateEntity();
    GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Green);
    GetWorld()->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
                                                                    Umbra::Math::Vector2f(0, -50),
                                                                    Umbra::Math::Vector2f(50, 50)));
    //   GetWorld()->Create
    //   GetWorld()->Create
}
