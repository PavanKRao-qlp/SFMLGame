#include "LDtkViewerScene.h"

#include "Core/Clock.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/Transform.h"
#include "Input/Input.h"

using namespace Umbra;

LDtkViewerScene::LDtkViewerScene(const String& _ldtkPath, const String& _levelName)
    : LDtkScene(_ldtkPath, _levelName), mPath(_ldtkPath), mLevel(_levelName) {}

void LDtkViewerScene::Initialize() {
    LDtkScene::Initialize();
    CreateDefaultCamera(300.0f);
}

void LDtkViewerScene::OnUpdate() {
    LDtkScene::OnUpdate();

    EntityID cam             = GetCameraEntity();
    World* world             = GetWorld();
    TransformComponent* tf   = world->GetComponent<TransformComponent>(cam);
    CameraComponent* camComp = world->GetComponent<CameraComponent>(cam);

    if (!tf || !camComp) {
        return;
    }

    float dt    = EngineTime::GetDeltaTime();
    float speed = camComp->GetOrthographicSize() * 2.0f;

    // Pan with WASD or arrow keys
    if (Input::GetKey(KeyBoard::W) || Input::GetKey(KeyBoard::Up)) {
        tf->Position.y -= speed * dt;
    }
    if (Input::GetKey(KeyBoard::S) || Input::GetKey(KeyBoard::Down)) {
        tf->Position.y += speed * dt;
    }
    if (Input::GetKey(KeyBoard::A) || Input::GetKey(KeyBoard::Left)) {
        tf->Position.x -= speed * dt;
    }
    if (Input::GetKey(KeyBoard::D) || Input::GetKey(KeyBoard::Right)) {
        tf->Position.x += speed * dt;
    }

    // Zoom with = (in) and - (out)
    float orthoSize = camComp->GetOrthographicSize();
    if (Input::GetKey(KeyBoard::Equal)) {
        camComp->SetOrthographicSize(std::max(10.f, orthoSize - orthoSize * 2.0f * dt));
    }
    if (Input::GetKey(KeyBoard::Hyphen)) {
        camComp->SetOrthographicSize(orthoSize + orthoSize * 2.0f * dt);
    }

    // if (Input::GetKeyDown(KeyBoard::Escape))
    //   GetGameInstance()->QuitApplication();
}

SharedPtr<Scene> LDtkViewerScene::InstantiateCopy() {
    return std::make_shared<LDtkViewerScene>(mPath, mLevel);
}
