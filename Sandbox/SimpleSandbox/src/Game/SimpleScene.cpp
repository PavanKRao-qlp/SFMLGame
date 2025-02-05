#include "SimpleScene.h"
#include "Umbra.h"
#include "Input/Input.h"
#include "Game/IGameInstance.h"

void SimpleScene::Initialize()
{
    UMBRA_LOG_INFO("SimpleScene Initialized !!");
    // AddSystem();
}

void SimpleScene::OnUpdate()
{
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape))
    {
        GetGameInstance()->QuitApplication();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Space))
    {
        // Scene *currentScene = GetGameInstance()->GetSceneManger()->GetCurrentScene();
        // GetGameInstance()->GetSceneManger()->GoToScene(currentScene->GetSceneID());
    }
}

void SimpleScene::OnBeginPlay()
{
    // auto BG = GetWorld()->CreateEntity();
}
