#include "NarrowPhaseScene.h"

#include "Core/Random.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/Transform.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Math/Bounds.h"
#include "Service/Physics/Collision.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

void NarrowPhaseScene::Initialize() {
    CreateDefaultCamera(200.0f);
}

void NarrowPhaseScene::OnFixedUpdate() {}

void NarrowPhaseScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetSceneManager().GoToScene("MainScene");
    }

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Narrow Phase Collisions");
    if (ImGui::Button("Back to Main Menu")) {
        GetSceneManager().GoToScene("MainScene");
    }
    ImGui::Separator();
    ImGui::Checkbox("Bounding Volumes", &bShowBoundingVolumes);
    ImGui::Checkbox("Contact Points", &bShowContactPoints);
    ImGui::Checkbox("Contact Normals", &bShowContactNormals);
    ImGui::Checkbox("Penetration Depth", &bShowPenetration);
    ImGui::Separator();
    ImGui::Text("Collisions: %d", mCollisionCount);
    ImGui::Text("Contact Points: %d", mContactPointCount);
    ImGui::End();

    MovePlayerBox();
    DrawColliderOutlines();
    if (bShowBoundingVolumes) {
        DrawBoundingVolumes();
    }
    DrawCollisionInfo();
}

Umbra::SharedPtr<Umbra::Scene> NarrowPhaseScene::InstantiateCopy() {
    return std::make_shared<NarrowPhaseScene>(*this);
}

void NarrowPhaseScene::OnBeginPlay() {
    Umbra::World* world = GetWorld();
    mEntities.clear();

    // --- Static floor ---
    {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(0.0f, -180.0f), Umbra::Math::Vector2f(400.0f, 20.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f; // static
        rb.bAffectedByGravity = false;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(400.0f, 20.0f));
    }

    // --- Static angled ramp ---
    {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(-100.0f, -120.0f), Umbra::Math::Vector2f(160.0f, 15.0f), 20.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(160.0f, 15.0f));
    }

    // --- Player-controlled kinematic box (WASD) ---
    {
        mPlayerBox = world->CreateEntity();
        mEntities.push_back(mPlayerBox);

        Umbra::Math::Vector2f size(40.0f, 40.0f);
        world->AddComponent<Umbra::TransformComponent>(mPlayerBox, Umbra::Math::Vector2f(0.0f, 0.0f), size, 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = false;
        rb.bIsKinematic       = true;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(mPlayerBox, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(mPlayerBox, size);
    }

    // --- Dynamic circles ---
    for (int i = 0; i < 4; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);

        float posX   = -60.0f + i * 40.0f;
        float posY   = 40.0f + Umbra::Random::RandomRange(0.0f, 80.0f);
        float radius = Umbra::Random::RandomRange(12.0f, 25.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(radius * 2, radius * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CachedVelocity =
            Umbra::Math::Vector2f(Umbra::Random::RandomRange(-20.0f, 20.0f), Umbra::Random::RandomRange(-10.0f, 10.0f));
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = radius;
        world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);
    }

    // --- Dynamic boxes ---
    for (int i = 0; i < 4; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);

        float posX  = -80.0f + i * 50.0f;
        float posY  = 100.0f + Umbra::Random::RandomRange(0.0f, 60.0f);
        float sizeX = Umbra::Random::RandomRange(20.0f, 45.0f);
        float sizeY = Umbra::Random::RandomRange(20.0f, 45.0f);
        float angle = Umbra::Random::RandomRange(-30.0f, 30.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(sizeX, sizeY), angle);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CachedVelocity =
            Umbra::Math::Vector2f(Umbra::Random::RandomRange(-15.0f, 15.0f), Umbra::Random::RandomRange(-5.0f, 5.0f));
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(sizeX, sizeY));
    }
}

void NarrowPhaseScene::OnEndPlay() {
    mEntities.clear();
}

void NarrowPhaseScene::MovePlayerBox() {
    Umbra::World* world                  = GetWorld();
    Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(mPlayerBox);
    if (transform == nullptr) {
        return;
    }

    float dt = Umbra::GEngineStatics.GameConfig->FixedDeltaTime;
    Umbra::Math::Vector2f direction(0.0f, 0.0f);
    if (Umbra::Input::GetKey(Umbra::KeyBoard::W)) {
        direction.y += 1.0f;
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::S)) {
        direction.y -= 1.0f;
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::A)) {
        direction.x -= 1.0f;
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::D)) {
        direction.x += 1.0f;
    }

    transform->Position += direction * mPlayerSpeed * dt;
}

void NarrowPhaseScene::DrawColliderOutlines() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    for (size_t i = 0; i < mEntities.size(); ++i) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
        if (rb == nullptr || !rb->Handle.IsValid()) {
            continue;
        }

        Umbra::PhysicsBodyData* physBody     = physicsService->GetBodyData(rb->Handle);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(mEntities[i]);
        if (physBody == nullptr || transform == nullptr) {
            continue;
        }

        Umbra::Color color = physBody->IsStatic() ? Umbra::Color(100, 100, 100) : Umbra::Color::White;
        if (mEntities[i] == mPlayerBox) {
            color = Umbra::Color::Green;
        }

        if (physBody->BodyShape.IsCircle()) {
            Umbra::RenderSystem::DebugDrawCircle(
                transform->Position, physBody->BodyShape.GetCircle().GetRadius(), false, color);
        } else if (physBody->BodyShape.IsBox()) {
            Umbra::Math::Bounds2D bounds(transform->Position, physBody->BodyShape.GetBox().GetSize());
            Umbra::RenderSystem::DrawDebugOrientedBox(bounds, transform->Angle, false, color);
        }
    }
}

void NarrowPhaseScene::DrawBoundingVolumes() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    for (size_t i = 0; i < mEntities.size(); ++i) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
        if (rb == nullptr || !rb->Handle.IsValid()) {
            continue;
        }

        Umbra::PhysicsBodyData* physBody     = physicsService->GetBodyData(rb->Handle);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(mEntities[i]);
        if (physBody == nullptr || transform == nullptr) {
            continue;
        }

        Umbra::Math::Bounds2D bounds(transform->Position, physBody->BoundingAABB.Size);
        Umbra::RenderSystem::DrawDebugBox(bounds, false, Umbra::Color::Red);
    }
}

void NarrowPhaseScene::DrawCollisionInfo() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();
    const auto& collisions                = physicsService->GetCollisions();

    mCollisionCount    = static_cast<int>(collisions.size());
    mContactPointCount = 0;

    const float normalLength  = 25.0f;
    const float contactRadius = 1.0f;

    for (const auto& collision : collisions) {
        mContactPointCount += static_cast<int>(collision.contacts.size());

        // Draw contact points
        if (bShowContactPoints) {
            for (const auto& contact : collision.contacts) {
                Umbra::RenderSystem::DebugDrawCircle(contact.contactPoint, contactRadius, true, Umbra::Color::Red);
            }
        }

        // Draw contact normals from each contact point
        if (bShowContactNormals) {
            for (const auto& contact : collision.contacts) {
                Umbra::Math::Vector2f normalEnd = contact.contactPoint + collision.contactNormal * normalLength;
                Umbra::RenderSystem::DebugDrawLine(contact.contactPoint, normalEnd, Umbra::Color::Yellow);
            }
        }

        // Draw penetration depth as a line along the normal from body A center
        if (bShowPenetration) {
            for (const auto& contact : collision.contacts) {
                Umbra::Math::Vector2f penEnd = contact.contactPoint + collision.contactNormal * collision.penetration;
                Umbra::RenderSystem::DebugDrawLine(contact.contactPoint, penEnd, Umbra::Color::Cyan);
            }
        }
    }
}
