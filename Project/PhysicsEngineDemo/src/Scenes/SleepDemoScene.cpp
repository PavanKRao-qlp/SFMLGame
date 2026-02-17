#include "SleepDemoScene.h"

#include "Core/Random.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/Transform.h"
#include "Service/ServiceLocator.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Math/Bounds.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

void SleepDemoScene::Initialize() {
    CreateDefaultCamera(200.0f);
}

void SleepDemoScene::OnFixedUpdate() {}

void SleepDemoScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetSceneManager().GoToScene("MainScene");
    }

    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    // --- ImGui Panel ---
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Sleep Demo");

    if (ImGui::Button("Back to Main Menu")) {
        GetSceneManager().GoToScene("MainScene");
    }
    ImGui::Separator();

    // --- Sleep Config ---
    if (ImGui::CollapsingHeader("Sleep Config", ImGuiTreeNodeFlags_DefaultOpen)) {
        Umbra::PhysicsServiceConfig& config = physicsService->GetConfig();
        ImGui::DragFloat("Linear Vel Threshold", &config.LinearVelocitySleepThreshold, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Angular Vel Threshold", &config.AngularVelocitySleepThreshold, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Sleep Time Threshold", &config.SleepTimeThreshold, 0.05f, 0.0f, 5.0f);
    }
    ImGui::Separator();

    // --- Stats ---
    int awakeCount   = 0;
    int sleepCount   = 0;
    CountSleepStats(awakeCount, sleepCount);
    int totalDynamic = awakeCount + sleepCount;

    ImGui::Text("Stats");
    ImGui::Text("Total Dynamic: %d", totalDynamic);
    ImGui::Text("Awake: %d", awakeCount);
    ImGui::Text("Sleeping: %d", sleepCount);
    ImGui::Separator();

    // --- Actions ---
    if (ImGui::Button("Drop Body")) {
        SpawnDynamicBody();
    }
    ImGui::SameLine();
    if (ImGui::Button("Wake All")) {
        for (size_t i = 0; i < mEntities.size(); ++i) {
            Umbra::RigidbodyHandleComponent* rb =
                world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
            if (rb == nullptr || !rb->Handle.IsValid()) {
                continue;
            }
            Umbra::PhysicsBodyData* body = physicsService->GetBodyData(rb->Handle);
            if (body != nullptr && !body->IsStatic() && !body->bIsKinematic) {
                body->Wake();
            }
        }
    }
    ImGui::Separator();

    // --- Toggles ---
    ImGui::Checkbox("Show Collider Outlines", &bShowColliders);

    ImGui::End();

    // --- Drawing ---
    MovePlayer();
    if (bShowColliders) {
        DrawColliderOutlines();
    }
}

Umbra::SharedPtr<Umbra::Scene> SleepDemoScene::InstantiateCopy() {
    return std::make_shared<SleepDemoScene>(*this);
}

void SleepDemoScene::OnBeginPlay() {
    Umbra::World* world = GetWorld();
    mEntities.clear();

    // --- Static floor ---
    {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(0.0f, -180.0f), Umbra::Math::Vector2f(400.0f, 20.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.2f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(400.0f, 20.0f));
    }

    // --- Left wall ---
    {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(-200.0f, 0.0f), Umbra::Math::Vector2f(20.0f, 380.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.2f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(20.0f, 380.0f));
    }

    // --- Right wall ---
    {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(200.0f, 0.0f), Umbra::Math::Vector2f(20.0f, 380.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.2f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(20.0f, 380.0f));
    }

    // --- WASD kinematic player ---
    {
        mPlayerEntity = world->CreateEntity();
        mEntities.push_back(mPlayerEntity);

        float radius = 15.0f;
        world->AddComponent<Umbra::TransformComponent>(
            mPlayerEntity, Umbra::Math::Vector2f(0.0f, -100.0f),
            Umbra::Math::Vector2f(radius * 2, radius * 2), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = false;
        rb.bIsKinematic       = true;
        rb.bCanSleep          = false;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(mPlayerEntity, rb);
        world->AddComponent<Umbra::CircleColliderComponent>(mPlayerEntity, radius);
    }

    // --- Dynamic circles ---
    for (int i = 0; i < 8; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);

        float posX   = Umbra::Random::RandomRange(-150.0f, 150.0f);
        float posY   = Umbra::Random::RandomRange(0.0f, 150.0f);
        float radius = Umbra::Random::RandomRange(8.0f, 18.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(radius * 2, radius * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = Umbra::Random::RandomRange(0.2f, 0.4f);
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = radius;
        world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);
    }

    // --- Dynamic boxes ---
    for (int i = 0; i < 7; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);

        float posX  = Umbra::Random::RandomRange(-150.0f, 150.0f);
        float posY  = Umbra::Random::RandomRange(0.0f, 150.0f);
        float sizeX = Umbra::Random::RandomRange(15.0f, 35.0f);
        float sizeY = Umbra::Random::RandomRange(15.0f, 35.0f);
        float angle = Umbra::Random::RandomRange(-20.0f, 20.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(sizeX, sizeY), angle);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = Umbra::Random::RandomRange(0.2f, 0.4f);
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(sizeX, sizeY));
    }

    // Read initial sleep config values
    const Umbra::PhysicsServiceConfig& config = world->GetPhysicsService()->GetConfig();
    mLinearSleepThreshold                     = config.LinearVelocitySleepThreshold;
    mAngularSleepThreshold                    = config.AngularVelocitySleepThreshold;
    mSleepTimeThreshold                       = config.SleepTimeThreshold;
}

void SleepDemoScene::OnEndPlay() {
    mEntities.clear();
}

void SleepDemoScene::MovePlayer() {
    Umbra::World* world                  = GetWorld();
    Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(mPlayerEntity);
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

void SleepDemoScene::SpawnDynamicBody() {
    Umbra::World* world    = GetWorld();
    Umbra::EntityID entity = world->CreateEntity();
    mEntities.push_back(entity);

    float posX    = Umbra::Random::RandomRange(-140.0f, 140.0f);
    float posY    = 160.0f;
    bool isCircle = Umbra::Random::RandomRange(0.0f, 1.0f) > 0.5f;

    if (isCircle) {
        float radius = Umbra::Random::RandomRange(8.0f, 18.0f);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(radius * 2, radius * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = Umbra::Random::RandomRange(0.2f, 0.4f);
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = radius;
        world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);
    } else {
        float sizeX = Umbra::Random::RandomRange(15.0f, 35.0f);
        float sizeY = Umbra::Random::RandomRange(15.0f, 35.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(sizeX, sizeY));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = Umbra::Random::RandomRange(0.2f, 0.4f);
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(sizeX, sizeY));
    }
}

void SleepDemoScene::DrawColliderOutlines() {
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

        // Color by state
        Umbra::Color color;
        if (mEntities[i] == mPlayerEntity) {
            color = Umbra::Color::Cyan;
        } else if (physBody->IsStatic()) {
            color = Umbra::Color(0.25f, 0.25f, 0.25f);
        } else if (physBody->bIsSleeping) {
            color = Umbra::Color(0.4f, 0.4f, 0.4f);
        } else {
            color = Umbra::Color::Green;
        }

        if (physBody->BodyShape.IsCircle()) {
            Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(
                transform->Position, physBody->BodyShape.GetCircle().GetRadius(), false, color);
        } else if (physBody->BodyShape.IsBox()) {
            Umbra::Math::Bounds2D bounds(transform->Position, physBody->BodyShape.GetBox().GetSize());
            Umbra::ServiceLocator::GetRenderService()->DebugDrawOrientedBox(bounds, transform->Angle, false, color);
        }
    }
}

void SleepDemoScene::CountSleepStats(int& _awake, int& _sleeping) {
    _awake    = 0;
    _sleeping = 0;

    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    for (size_t i = 0; i < mEntities.size(); ++i) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
        if (rb == nullptr || !rb->Handle.IsValid()) {
            continue;
        }

        Umbra::PhysicsBodyData* physBody = physicsService->GetBodyData(rb->Handle);
        if (physBody == nullptr || physBody->IsStatic() || physBody->bIsKinematic) {
            continue;
        }

        if (physBody->bIsSleeping) {
            ++_sleeping;
        } else {
            ++_awake;
        }
    }
}
