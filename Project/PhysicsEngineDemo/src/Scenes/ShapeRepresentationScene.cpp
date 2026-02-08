#include "ShapeRepresentationScene.h"

#include "Core/Random.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Math/Bounds.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

void ShapeRepresentationScene::Initialize() {
    CreateDefaultCamera(200.0f);
}

void ShapeRepresentationScene::OnBeginPlay() {
    Umbra::World* world = GetWorld();
    mEntities.clear();

    // Spawn 8 entities in a grid layout
    const int numEntities = 8;
    const int cols        = 4;
    const float spacing   = 60.0f;
    const float startX    = -spacing * (cols - 1) * 0.5f;
    const float startY    = -spacing * 0.5f;

    for (int i = 0; i < numEntities; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);

        int col    = i % cols;
        int row    = i / cols;
        float posX = startX + col * spacing + Umbra::Random::RandomRange(-10.0f, 10.0f);
        float posY = startY + row * spacing + Umbra::Random::RandomRange(-10.0f, 10.0f);

        float size = Umbra::Random::RandomRange(20.0f, 40.0f);
        world->AddComponent<Umbra::TransformComponent>(entity, Umbra::Math::Vector2f(posX, posY),
            Umbra::Math::Vector2f(size, size), Umbra::Random::RandomRange(-360, 360));

        Umbra::Color color(Umbra::Random::RandomRange(0.3f, 1.0f), Umbra::Random::RandomRange(0.3f, 1.0f),
            Umbra::Random::RandomRange(0.3f, 1.0f), 0.5f);
        world->AddComponent<Umbra::SpriteComponent>(entity, color);

        // Rigidbody: no gravity, low random velocity
        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = false;
        rb.CachedVelocity =
            Umbra::Math::Vector2f(Umbra::Random::RandomRange(-15.0f, 15.0f), Umbra::Random::RandomRange(-15.0f, 15.0f));
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        // Randomly assign circle or box collider
        bool bUseCircle = Umbra::Random::RandomRange(0, 1) == 0;
        if (bUseCircle) {
            Umbra::CircleColliderComponent circle;
            circle.Radius = size * 0.5f;
            world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);
        } else {
            world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(size, size));
        }
    }
}

void ShapeRepresentationScene::OnFixedUpdate() {}

void ShapeRepresentationScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetSceneManager().GoToScene("MainScene");
    }

    // ImGui UI
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Shape Representation");
    if (ImGui::Button("Back to Main Menu")) {
        GetSceneManager().GoToScene("MainScene");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> ShapeRepresentationScene::InstantiateCopy() {
    return std::make_shared<ShapeRepresentationScene>(*this);
}

void ShapeRepresentationScene::OnEndPlay() {
    mEntities.clear();
}
