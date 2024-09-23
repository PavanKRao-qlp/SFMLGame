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

    UMBRA::EntityID ballB = mWorldRegister->CreateEntity();
    mWorldRegister->AddComponent<UMBRA::TransformComponent>(ballB, UMBRA::TransformComponent(UMBRA::MATH::Vector2f(600, 400), UMBRA::MATH::Vector2f(100, 100)));
    mWorldRegister->AddComponent<UMBRA::SpriteComponent>(ballB, sf::Color::Blue);
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
    if (UMBRA::Input::GetMouseButtonDown(UMBRA::Mouse::Left))
    {
        UMBRA::EntityID ballA = mWorldRegister->CreateEntity();
        mWorldRegister->AddComponent<UMBRA::TransformComponent>(ballA, UMBRA::Input::GetMousePosition(), UMBRA::MATH::Vector2f(10, 10));
        mWorldRegister->AddComponent<UMBRA::SpriteComponent>(ballA, sf::Color::Red);
        mWorldRegister->AddComponent<UMBRA::LifeTimeComponent>(ballA, 5.f);
    };
}