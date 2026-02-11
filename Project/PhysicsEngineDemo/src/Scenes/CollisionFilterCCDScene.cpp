#include "CollisionFilterCCDScene.h"

#include "Core/Random.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/Transform.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Math/Bounds.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

// ────────────────────────────────────────────────────────────────
// Helpers
// ────────────────────────────────────────────────────────────────

static Umbra::Color GroupColor(CollisionFilterCCDScene::EFilterGroup _group) {
    switch (_group) {
    case CollisionFilterCCDScene::EFilterGroup::Red:   return Umbra::Color::Red;
    case CollisionFilterCCDScene::EFilterGroup::Green:  return Umbra::Color::Green;
    case CollisionFilterCCDScene::EFilterGroup::Blue:   return Umbra::Color::Blue;
    default: return Umbra::Color::White;
    }
}

static uint16_t GroupCategory(CollisionFilterCCDScene::EFilterGroup _group) {
    switch (_group) {
    case CollisionFilterCCDScene::EFilterGroup::Red:   return 0x0002;
    case CollisionFilterCCDScene::EFilterGroup::Green:  return 0x0004;
    case CollisionFilterCCDScene::EFilterGroup::Blue:   return 0x0008;
    default: return 0x0001;
    }
}

// ────────────────────────────────────────────────────────────────
// Scene lifecycle
// ────────────────────────────────────────────────────────────────

void CollisionFilterCCDScene::Initialize() {
    CreateDefaultCamera(250.0f);
}

void CollisionFilterCCDScene::OnFixedUpdate() {}

void CollisionFilterCCDScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetSceneManager().GoToScene("MainScene");
    }

    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    // ── ImGui Panel ──────────────────────────────────────────────
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Collision Filter & CCD Demo");

    if (ImGui::Button("Back to Main Menu")) {
        GetSceneManager().GoToScene("MainScene");
    }
    ImGui::Separator();

    // ── Filter toggles ──────────────────────────────────────────
    if (ImGui::CollapsingHeader("Collision Filtering", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped(
            "Each colored group has its own category bit. "
            "Toggle which groups collide with each other.");

        bool filtersChanged = false;
        filtersChanged |= ImGui::Checkbox("Red <-> Green", &bRedCollidesGreen);
        filtersChanged |= ImGui::Checkbox("Red <-> Blue", &bRedCollidesBlue);
        filtersChanged |= ImGui::Checkbox("Green <-> Blue", &bGreenCollidesBlue);

        if (filtersChanged) {
            // Rebuild mask bits for every dynamic body based on toggles
            for (auto& info : mDynamicEntities) {
                Umbra::RigidbodyHandleComponent* rb =
                    world->GetComponent<Umbra::RigidbodyHandleComponent>(info.Entity);
                if (rb == nullptr || !rb->Handle.IsValid()) {
                    continue;
                }

                uint16_t category = GroupCategory(info.Group);
                uint16_t mask     = CATEGORY_DEFAULT | CATEGORY_BULLET | category; // always self + walls + bullets

                // Add cross-group masks
                if (info.Group == EFilterGroup::Red) {
                    if (bRedCollidesGreen) mask |= CATEGORY_GREEN;
                    if (bRedCollidesBlue)  mask |= CATEGORY_BLUE;
                }
                if (info.Group == EFilterGroup::Green) {
                    if (bRedCollidesGreen) mask |= CATEGORY_RED;
                    if (bGreenCollidesBlue) mask |= CATEGORY_BLUE;
                }
                if (info.Group == EFilterGroup::Blue) {
                    if (bRedCollidesBlue)   mask |= CATEGORY_RED;
                    if (bGreenCollidesBlue) mask |= CATEGORY_GREEN;
                }

                Umbra::CollisionFilter filter;
                filter.CategoryBits = category;
                filter.MaskBits     = mask;
                physicsService->SetCollisionFilter(rb->Handle, filter);
            }
        }
    }
    ImGui::Separator();

    // ── CCD section ─────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Continuous Collision Detection", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped(
            "A fast-moving bullet is fired across the scene. "
            "Toggle CCD to see the difference: without CCD the "
            "bullet tunnels through thin objects.");
        ImGui::Checkbox("Enable CCD on Bullets", &bBulletCCDEnabled);
        ImGui::DragFloat("Bullet Speed", &mBulletSpeed, 50.0f, 500.0f, 10000.0f);

        if (ImGui::Button("Fire Bullet ->")) {
            SpawnBullet();
        }
    }
    ImGui::Separator();

    ImGui::Checkbox("Show Collider Outlines", &bShowColliders);

    ImGui::Text("Bullets alive: %d", (int)mBulletEntities.size());

    ImGui::End();

    // ── Drawing ─────────────────────────────────────────────────
    if (bShowColliders) {
        DrawColliderOutlines();
    }

    // Clean up bullets that have flown off-screen
    for (int i = (int)mBulletEntities.size() - 1; i >= 0; --i) {
        Umbra::TransformComponent* t = world->GetComponent<Umbra::TransformComponent>(mBulletEntities[i]);
        if (t == nullptr) {
            continue;
        }
        if (t->Position.x > 350.0f || t->Position.x < -350.0f ||
            t->Position.y > 350.0f || t->Position.y < -350.0f) {
            world->DestroyEntity(mBulletEntities[i]);
            mBulletEntities.erase(mBulletEntities.begin() + i);
        }
    }
}

Umbra::SharedPtr<Umbra::Scene> CollisionFilterCCDScene::InstantiateCopy() {
    return std::make_shared<CollisionFilterCCDScene>(*this);
}

void CollisionFilterCCDScene::OnBeginPlay() {
    Umbra::World* world = GetWorld();
    mDynamicEntities.clear();
    mStaticEntities.clear();
    mBulletEntities.clear();

    // ── Static floor ─────────────────────────────────────────────
    {
        Umbra::EntityID entity = world->CreateEntity();
        mStaticEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(0.0f, -220.0f), Umbra::Math::Vector2f(500.0f, 20.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.2f;
        rb.Filter.CategoryBits = CATEGORY_DEFAULT;
        rb.Filter.MaskBits     = 0xFFFF; // walls collide with everything
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(500.0f, 20.0f));
    }

    // ── Left wall ────────────────────────────────────────────────
    {
        Umbra::EntityID entity = world->CreateEntity();
        mStaticEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(-250.0f, 0.0f), Umbra::Math::Vector2f(20.0f, 460.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.2f;
        rb.Filter.CategoryBits = CATEGORY_DEFAULT;
        rb.Filter.MaskBits     = 0xFFFF;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(20.0f, 460.0f));
    }

    // ── Right wall ───────────────────────────────────────────────
    {
        Umbra::EntityID entity = world->CreateEntity();
        mStaticEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(250.0f, 0.0f), Umbra::Math::Vector2f(20.0f, 460.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.2f;
        rb.Filter.CategoryBits = CATEGORY_DEFAULT;
        rb.Filter.MaskBits     = 0xFFFF;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(20.0f, 460.0f));
    }

    // ── Thin vertical wall (CCD target) ─────────────────────────
    // A thin wall in the middle - fast bullets will tunnel through without CCD
    {
        Umbra::EntityID entity = world->CreateEntity();
        mStaticEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(100.0f, -50.0f), Umbra::Math::Vector2f(4.0f, 150.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.5f;
        rb.Filter.CategoryBits = CATEGORY_DEFAULT;
        rb.Filter.MaskBits     = 0xFFFF;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(4.0f, 150.0f));
    }

    // ── Red group (left column) ──────────────────────────────────
    for (int i = 0; i < 5; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        float posX             = Umbra::Random::RandomRange(-200.0f, -100.0f);
        float posY             = Umbra::Random::RandomRange(-150.0f, 150.0f);
        float radius           = Umbra::Random::RandomRange(10.0f, 20.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(radius * 2, radius * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = 0.4f;
        rb.Filter.CategoryBits = CATEGORY_RED;
        rb.Filter.MaskBits     = CATEGORY_DEFAULT | CATEGORY_RED | CATEGORY_BULLET;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = radius;
        world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);

        mDynamicEntities.push_back({entity, EFilterGroup::Red});
    }

    // ── Green group (center column) ──────────────────────────────
    for (int i = 0; i < 5; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        float posX             = Umbra::Random::RandomRange(-50.0f, 50.0f);
        float posY             = Umbra::Random::RandomRange(-150.0f, 150.0f);
        float sizeX            = Umbra::Random::RandomRange(15.0f, 30.0f);
        float sizeY            = Umbra::Random::RandomRange(15.0f, 30.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(sizeX, sizeY));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = 0.4f;
        rb.Filter.CategoryBits = CATEGORY_GREEN;
        rb.Filter.MaskBits     = CATEGORY_DEFAULT | CATEGORY_GREEN | CATEGORY_BULLET;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(sizeX, sizeY));

        mDynamicEntities.push_back({entity, EFilterGroup::Green});
    }

    // ── Blue group (right column) ────────────────────────────────
    for (int i = 0; i < 5; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        float posX             = Umbra::Random::RandomRange(120.0f, 220.0f);
        float posY             = Umbra::Random::RandomRange(-150.0f, 150.0f);
        float radius           = Umbra::Random::RandomRange(10.0f, 20.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(radius * 2, radius * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = 0.4f;
        rb.Filter.CategoryBits = CATEGORY_BLUE;
        rb.Filter.MaskBits     = CATEGORY_DEFAULT | CATEGORY_BLUE | CATEGORY_BULLET;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = radius;
        world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);

        mDynamicEntities.push_back({entity, EFilterGroup::Blue});
    }
}

void CollisionFilterCCDScene::OnEndPlay() {
    mDynamicEntities.clear();
    mStaticEntities.clear();
    mBulletEntities.clear();
}

// ────────────────────────────────────────────────────────────────
// Bullet spawning (CCD demo)
// ────────────────────────────────────────────────────────────────

void CollisionFilterCCDScene::SpawnBullet() {
    Umbra::World* world    = GetWorld();
    Umbra::EntityID entity = world->CreateEntity();
    mBulletEntities.push_back(entity);

    float radius = 3.0f;
    float spawnX = -230.0f;
    float spawnY = Umbra::Random::RandomRange(-100.0f, 0.0f);

    world->AddComponent<Umbra::TransformComponent>(
        entity, Umbra::Math::Vector2f(spawnX, spawnY), Umbra::Math::Vector2f(radius * 2, radius * 2));

    Umbra::RigidbodyHandleComponent rb;
    rb.Mass               = 0.5f;
    rb.bAffectedByGravity = false;
    rb.bCanSleep          = false;
    rb.CoefOfRestitution  = 1.0f;
    rb.bEnableCCD         = bBulletCCDEnabled;
    rb.Filter.CategoryBits = CATEGORY_BULLET;
    rb.Filter.MaskBits     = 0xFFFF; // bullets collide with everything
    rb.CachedVelocity      = Umbra::Math::Vector2f(mBulletSpeed, 0.0f);
    world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

    Umbra::CircleColliderComponent circle;
    circle.Radius = radius;
    world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);
}

// ────────────────────────────────────────────────────────────────
// Debug drawing
// ────────────────────────────────────────────────────────────────

void CollisionFilterCCDScene::DrawColliderOutlines() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    // Draw static walls
    for (auto entity : mStaticEntities) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(entity);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(entity);
        if (rb == nullptr || transform == nullptr || !rb->Handle.IsValid()) {
            continue;
        }
        Umbra::PhysicsBodyData* physBody = physicsService->GetBodyData(rb->Handle);
        if (physBody == nullptr) {
            continue;
        }

        Umbra::Color color(0.3f, 0.3f, 0.3f);
        if (physBody->BodyShape.IsBox()) {
            Umbra::Math::Bounds2D bounds(transform->Position, physBody->BodyShape.GetBox().GetSize());
            Umbra::RenderSystem::DrawDebugOrientedBox(bounds, transform->Angle, false, color);
        }
    }

    // Draw dynamic bodies colored by group
    for (auto& info : mDynamicEntities) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(info.Entity);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(info.Entity);
        if (rb == nullptr || transform == nullptr || !rb->Handle.IsValid()) {
            continue;
        }
        Umbra::PhysicsBodyData* physBody = physicsService->GetBodyData(rb->Handle);
        if (physBody == nullptr) {
            continue;
        }

        Umbra::Color color = GroupColor(info.Group);

        if (physBody->BodyShape.IsCircle()) {
            Umbra::RenderSystem::DebugDrawCircle(
                transform->Position, physBody->BodyShape.GetCircle().GetRadius(), true, color);
        } else if (physBody->BodyShape.IsBox()) {
            Umbra::Math::Bounds2D bounds(transform->Position, physBody->BodyShape.GetBox().GetSize());
            Umbra::RenderSystem::DrawDebugOrientedBox(bounds, transform->Angle, true, color);
        }
    }

    // Draw bullets
    for (auto entity : mBulletEntities) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(entity);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(entity);
        if (rb == nullptr || transform == nullptr || !rb->Handle.IsValid()) {
            continue;
        }
        Umbra::PhysicsBodyData* physBody = physicsService->GetBodyData(rb->Handle);
        if (physBody == nullptr) {
            continue;
        }

        Umbra::Color color = Umbra::Color::Yellow;
        if (physBody->BodyShape.IsCircle()) {
            Umbra::RenderSystem::DebugDrawCircle(
                transform->Position, physBody->BodyShape.GetCircle().GetRadius(), true, color);
        }
    }
}
