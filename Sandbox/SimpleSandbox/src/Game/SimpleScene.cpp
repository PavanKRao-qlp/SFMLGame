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
}

void SimpleScene::OnBeginPlay()
{
    Umbra::EntityID entity = GetWorld()->CreateEntity();
    GetWorld()->AddComponent<Umbra::SpriteComponent>(entity);
    GetWorld()->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
                                                                    Umbra::Math::Vector2f(0, 0),
                                                                    Umbra::Math::Vector2f(50, 50)));
    //   GetWorld()->Create
}
