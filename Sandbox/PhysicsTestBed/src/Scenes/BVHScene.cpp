#include "BVHScene.h"

#include "Core/Random.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "LandingScene.h"
#include "Math/GeometryUtils.h"
#include "PointLineSegmentScene.h"
#include "Umbra.h"
#include "Graphics/Color.h"
#include "imgui.h"

void BVHScene::Initialize() {
    CreateDefaultCamera(mAreaSize);
}

void BVHScene::OnFixedUpdate() {
    Umbra::Math::Vector2f worldMousePos = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    Umbra::Math::Bounds2D mouseBound    = Umbra::Math::Bounds2D(worldMousePos, mBoxSize);
    Umbra::Math::Ray2D mouseRay =
        Umbra::Math::Ray2D(worldMousePos, (Umbra::Math::Vector2f(1, 0).GetRotated(mRayAngle)));
    // if (GetWorld()->GetPhysicsSystem()->QueryColliderAt(worldMousePos)) {
    //     mPointColliding = true;
    // } else {
    //     mPointColliding = false;
    // }
    // if (GetWorld()->GetPhysicsSystem()->QueryCollidersInsideAABB(mouseBound)) {
    //     mAABBColliding = true;
    // } else {
    //     mAABBColliding = false;
    // }
    // if (GetWorld()->GetPhysicsSystem()->Raycast(mouseRay)) {
    //     mRayCastColliding = true;
    // } else {
    //     mRayCastColliding = false;
    // }

    Umbra::RenderSystem::DrawDebugBox(mouseBound, false, Umbra::Color::Cyan);
    Umbra::RenderSystem::DebugDrawLine(worldMousePos, worldMousePos + mouseRay.Direction * 10000, Umbra::Color::Cyan);
}

void BVHScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Right)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.x +=
                10 * Umbra::EngineTime::GetDeltaTime();
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Left)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.x -=
                10 * Umbra::EngineTime::GetDeltaTime();
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Up)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.y +=
                10 * Umbra::EngineTime::GetDeltaTime();
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Down)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.y -=
                10 * Umbra::EngineTime::GetDeltaTime();
        }
    }

    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Q)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            float orthographic =
                GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->GetOrthographicSize();
            orthographic += 40 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::E)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            float orthographic =
                GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->GetOrthographicSize();
            orthographic -= 40 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
    ImGui::Begin("BVH Demo");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    if (ImGui::Checkbox("is colliding with mouse", &mPointColliding)) {
    }
    if (ImGui::Checkbox("AABB query", &mAABBColliding)) {
    }
    if (ImGui::Checkbox("raycast hit query", &mRayCastColliding)) {
    }
    float pos[] = {mBoxSize.x, mBoxSize.y};
    if (ImGui::InputFloat2("Position", pos)) {
        mBoxSize.x = pos[0];
        mBoxSize.y = pos[1];
    }
    if (ImGui::SliderFloat("Ray angle", &mRayAngle, 0, 360)) {
    }
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> BVHScene::InstantiateCopy() {
    return std::make_shared<BVHScene>(*this);
}

void BVHScene::OnBeginPlay() {

    for (int i = 0; i < 10; i++) {
        int transformX = Umbra::Random::RandomRange(mAreaSize * -0.9f, mAreaSize * 0.9f);
        int transformY = Umbra::Random::RandomRange(mAreaSize * -0.9f, mAreaSize * 0.9f);

        int sizeRX = Umbra::Random::RandomRange(10, 30);
        int sizeRY = Umbra::Random::RandomRange(10, 30);

        float angle                = Umbra::Random::RandomRange(0, 360);
        Umbra::EntityID mBoxEntity = GetWorld()->CreateEntity();

        GetWorld()->AddComponent<Umbra::TransformComponent>(
            mBoxEntity, Umbra::Math::Vector2f(transformX, transformY), Umbra::Math::Vector2f(sizeRX, sizeRY), angle);
        GetWorld()->AddComponent<Umbra::BoxColliderComponent>(mBoxEntity, Umbra::Math::Vector2f(sizeRX, sizeRY));
        // Umbra::PhysicsBodyComponent physicsBodyComponent;
        // physicsBodyComponent.mAngularVelocity = Umbra::Random::RandomRange(-5, 5);
        // GetWorld()->AddComponent<Umbra::PhysicsBodyComponent>(mBoxEntity, physicsBodyComponent);
        GetWorld()->AddComponent<Umbra::SpriteComponent>(mBoxEntity, Umbra::Color::Red);
    }
    int sizeRX = Umbra::Random::RandomRange(10, 20);
    int sizeRY = Umbra::Random::RandomRange(10, 20);
    mBoxSize   = Umbra::Math::Vector2f(sizeRX, sizeRY);
}

void BVHScene::OnEndPlay() {}


// Add Entity with collider
// For each entity with collider
// get bounding box
// Add Bounding box transformed to Collision Detector
//
