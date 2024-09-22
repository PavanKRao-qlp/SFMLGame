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

    D2D::EntityID ballB = mWorldRegister->CreateEntity();
    mWorldRegister->AddComponent<D2D::TransformComponent>(ballB, D2D::TransformComponent(650, 100));
    mWorldRegister->AddComponent<D2D::SpriteComponent>(ballB, sf::Color::Blue);
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
    if (D2D::Input::GetMouseButtonDown(D2D::Mouse::Left))
    {
        D2D::EntityID ballA = mWorldRegister->CreateEntity();
        mWorldRegister->AddComponent<D2D::TransformComponent>(ballA, D2D::Input::GetMousePositionX(),  D2D::Input::GetMousePositionY());
        mWorldRegister->AddComponent<D2D::SpriteComponent>(ballA, sf::Color::Red);
        mWorldRegister->AddComponent<D2D::LifeTimeComponent>(ballA , 5);
    };
}