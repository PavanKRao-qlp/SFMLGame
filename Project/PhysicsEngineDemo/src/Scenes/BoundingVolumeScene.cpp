#include "BoundingVolumeScene.h"

#include "Core/Random.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "Service/ServiceLocator.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Math/Bounds.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

void BoundingVolumeScene::Initialize() {
    CreateDefaultCamera(200.0f);
}

void BoundingVolumeScene::OnFixedUpdate() {}

void BoundingVolumeScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetSceneManager().GoToScene("MainScene");
    }

    // ImGui UI
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Bounding Volume Representation");
    if (ImGui::Button("Back to Main Menu")) {
        GetSceneManager().GoToScene("MainScene");
    }
    ImGui::Separator();
    ImGui::Checkbox("Show bounding volume", &bShowBounds);
    ImGui::End();

    DrawColliderOutlines();
    if (bShowBounds) {
        DrawBoundsOutlines();
    }
}

Umbra::SharedPtr<Umbra::Scene> BoundingVolumeScene::InstantiateCopy() {
    return std::make_shared<BoundingVolumeScene>(*this);
}

void BoundingVolumeScene::OnBeginPlay() {
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

        float sizeX = Umbra::Random::RandomRange(20.0f, 80.0f);
        float sizeY = Umbra::Random::RandomRange(20.0f, 80.0f);
        world->AddComponent<Umbra::TransformComponent>(entity, Umbra::Math::Vector2f(posX, posY),
            Umbra::Math::Vector2f(sizeX, sizeY), Umbra::Random::RandomRange(-360, 360));

        // Umbra::Color color(Umbra::Random::RandomRange(0.3f, 1.0f), Umbra::Random::RandomRange(0.3f, 1.0f),
        //     Umbra::Random::RandomRange(0.3f, 1.0f), 0.5f);
        // world->AddComponent<Umbra::SpriteComponent>(entity, color);

        // Rigidbody: no gravity, low random velocity
        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = false;
        rb.CachedVelocity =
            Umbra::Math::Vector2f(Umbra::Random::RandomRange(-15.0f, 15.0f), Umbra::Random::RandomRange(-15.0f, 15.0f));
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        // Randomly assign circle or box collider
        bool bUseCircle = true;
        if (bUseCircle) {
            Umbra::CircleColliderComponent circle;
            circle.Radius = sizeX * 0.5f;
            world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);
        } else {
            world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(sizeX, sizeY));
        }
    }
}

void BoundingVolumeScene::OnEndPlay() {
    mEntities.clear();
}

void BoundingVolumeScene::DrawColliderOutlines() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    for (size_t i = 0; i < mEntities.size(); ++i) {
        Umbra::RigidbodyHandleComponent* rb  = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(mEntities[i]);
        if (!rb->Handle.IsValid()) {
            continue;
        }
        Umbra::PhysicsBodyData* physBody = physicsService->GetBodyData(rb->Handle);
        if (physBody->BodyShape.IsBox()) {
            Umbra::Math::Bounds2D bounds(transform->Position, physBody->BodyShape.GetBox().GetSize());
            Umbra::ServiceLocator::GetRenderService()->DebugDrawOrientedBox(bounds, 0, true);
        }
        if (physBody->BodyShape.IsCircle()) {
            Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(
                transform->Position, physBody->BodyShape.GetCircle().GetRadius(), true);
        }
    }
    // for (size_t j = i + 1; j < mEntities.size(); ++j) {
    //     = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[j]);

    //     if (rbA == nullptr || rbB == nullptr) {
    //         continue;
    //     }
    //     if (!rbA->HasValidBody() || !rbB->HasValidBody()) {
    //         continue;
    //     }
    //     // if (physicsService->TestOverlap(rbA->Handle, rbB->Handle)) {
    //     //     overlapping.insert(mEntities[i]);
    //     //     overlapping.insert(mEntities[j]);
    //     // }
    // }
    // // Draw collider outlines
    // for (Umbra::EntityID entity : mEntities) {
    //     Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(entity);
    //     if (transform == nullptr) {
    //         continue;
    //     }

    //     bool bIsOverlapping = overlapping.count(entity) > 0;
    //     Umbra::Color color  = bIsOverlapping ? Umbra::Color::Green : Umbra::Color::White;

    //     if (world->HasComponent<Umbra::CircleColliderComponent>(entity)) {
    //         Umbra::CircleColliderComponent* circle = world->GetComponent<Umbra::CircleColliderComponent>(entity);
    //         Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(transform->Position + circle->Offset, circle->Radius, false, color);
    //     } else if (world->HasComponent<Umbra::BoxColliderComponent>(entity)) {
    //         Umbra::BoxColliderComponent* box = world->GetComponent<Umbra::BoxColliderComponent>(entity);
    //         Umbra::Math::Bounds2D bounds(transform->Position + box->Offset, box->Size);
    //         Umbra::ServiceLocator::GetRenderService()->DebugDrawOrientedBox(bounds, transform->Angle, false, color);
    //     }
    // }
}

void BoundingVolumeScene::DrawBoundsOutlines() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    Umbra::Set<Umbra::EntityID> overlapping;
    for (size_t i = 0; i < mEntities.size(); ++i) {
        Umbra::RigidbodyHandleComponent* rb  = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(mEntities[i]);
        if (!rb->Handle.IsValid()) {
            continue;
        }
        Umbra::PhysicsBodyData* physBody = physicsService->GetBodyData(rb->Handle);
        Umbra::Math::Bounds2D bounds(transform->Position, physBody->BoundingAABB.Size);
        Umbra::ServiceLocator::GetRenderService()->DebugDrawOrientedBox(bounds, 0, false, Umbra::Color::Red);
    }
}
