#include "SimpleGameInstance.h"
#include "Input/Input.h"
#include "ECS/ECSRegister.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/LifeTime.h"
#include "SFML/Graphics.hpp"
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

    Umbra::EntityID ballB = mWorldRegister->CreateEntity();
    mWorldRegister->AddComponent<Umbra::TransformComponent>(ballB, Umbra::TransformComponent(Umbra::MATH::Vector2f(600, 400), Umbra::MATH::Vector2f(100, 100)));
    mWorldRegister->AddComponent<Umbra::SpriteComponent>(ballB, sf::Color::Blue);
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
    if (Umbra::Input::GetMouseButtonDown(Umbra::Mouse::Left))
    {
        Umbra::EntityID ballA = mWorldRegister->CreateEntity();
        mWorldRegister->AddComponent<Umbra::TransformComponent>(ballA, Umbra::Input::GetMousePosition(), Umbra::MATH::Vector2f(10, 10));
        mWorldRegister->AddComponent<Umbra::SpriteComponent>(ballA, sf::Color::Red);
        mWorldRegister->AddComponent<Umbra::LifeTimeComponent>(ballA, 5.f);
    };
}